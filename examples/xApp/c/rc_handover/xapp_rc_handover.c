/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include "../../../../src/xApp/e42_xapp_api.h"
#include "../../../../src/sm/rc_sm/ie/rc_data_ie.h"
#include "../../../../src/sm/rc_sm/rc_sm_id.h"
#include "../../../../src/sm/rc_sm/ie/ir/ran_param_struct.h"
#include "../../../../src/util/alg_ds/alg/defer.h"
#include "../../../../src/util/conversions.h"

#include "aper_decoder.h"
#include "F1AP_F1AP-PDU.h"
#include "F1AP_InitiatingMessage.h"
#include "F1AP_ProtocolIE-Field.h"

#include <inttypes.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// E2SM-RC 7.6.4: Connected mode mobility control, Handover Control.
static const uint32_t RC_CTRL_STYLE_CONN_MODE_MOBILITY = 3;

// E2SM-RC 7.4.6: On Demand Report.
static const uint32_t RC_REPORT_STYLE_ON_DEMAND = 5;

/* A UE and its currently reported serving cell, from a REPORT Style 5 On
 * Demand Report indication (sm_cb_rc). */
typedef struct {
  ue_id_e2sm_t ue_id;
  nr_cgi_t cell_global_id;
} ue_cell_t;

static ue_cell_t ue_cell = {0};

/* One decoded F1 Setup Request: the gNB-DU ID and the raw NR CGI of its served cell. */
typedef struct {
  uint64_t gnb_du_id;
  byte_array_t nr_cgi;
} f1_du_t;

static
void free_f1_dus(f1_du_t* dus, size_t len)
{
  for (size_t i = 0; i < len; i++)
    free_byte_array(dus[i].nr_cgi);
  free(dus);
}

/* For logging only; the CGI itself is used raw everywhere else. */
static
uint64_t nr_cgi_cell_id(const byte_array_t* nr_cgi)
{
  const uint8_t* b = nr_cgi->buf;
  return ((uint64_t)b[3] << 28) | ((uint64_t)b[4] << 20) | ((uint64_t)b[5] << 12) | ((uint64_t)b[6] << 4) | ((uint64_t)b[7] >> 4);
}

static
nr_cgi_t decode_nr_cgi_plmn_cell(const byte_array_t* nr_cgi)
{
  nr_cgi_t dst = {0};

  int mcc, mnc, mnc_digit_len;
  PLMNID_TO_MCC_MNC(nr_cgi, mcc, mnc, mnc_digit_len);
  dst.plmn_id.mcc = mcc;
  dst.plmn_id.mnc = mnc;
  dst.plmn_id.mnc_digit_len = mnc_digit_len;

  dst.nr_cell_id = nr_cgi_cell_id(nr_cgi);

  return dst;
}

// Wrap one RAN parameter in a single-element STRUCTURE value.
static
ran_param_val_type_t wrap_in_struct(seq_ran_param_t inner)
{
  ran_param_val_type_t dst = {.type = STRUCTURE_RAN_PARAMETER_VAL_TYPE};

  dst.strct = calloc(1, sizeof(ran_param_struct_t));
  assert(dst.strct != NULL && "Memory exhausted");

  dst.strct->sz_ran_param_struct = 1;
  dst.strct->ran_param_struct = calloc(1, sizeof(seq_ran_param_t));
  assert(dst.strct->ran_param_struct != NULL && "Memory exhausted");
  dst.strct->ran_param_struct[0] = inner;

  return dst;
}

/* Target Primary Cell ID > CHOICE Target Cell > NR Cell ID > NR CGI, the
 * nested shape 8.4.4.1 prescribes for Handover Control. Takes ownership of
 * nr_cgi's buffer. */
static
seq_ran_param_t fill_target_primary_cell_id(byte_array_t nr_cgi)
{
  seq_ran_param_t nr_cgi_p = {.ran_param_id = NR_CGI_8_4_4_1};
  nr_cgi_p.ran_param_val.type = ELEMENT_KEY_FLAG_FALSE_RAN_PARAMETER_VAL_TYPE;
  nr_cgi_p.ran_param_val.flag_false = calloc(1, sizeof(ran_parameter_value_t));
  assert(nr_cgi_p.ran_param_val.flag_false != NULL && "Memory exhausted");
  nr_cgi_p.ran_param_val.flag_false->type = OCTET_STRING_RAN_PARAMETER_VALUE;
  nr_cgi_p.ran_param_val.flag_false->octet_str_ran = nr_cgi;

  seq_ran_param_t nr_cell_id_p = {.ran_param_id = NR_CELL_8_4_4_1};
  nr_cell_id_p.ran_param_val = wrap_in_struct(nr_cgi_p);

  seq_ran_param_t choice_target_cell = {.ran_param_id = CHOICE_TARGET_CELL_8_4_4_1};
  choice_target_cell.ran_param_val = wrap_in_struct(nr_cell_id_p);

  seq_ran_param_t dst = {.ran_param_id = TARGET_PRIMARY_CELL_ID_8_4_4_1};
  dst.ran_param_val = wrap_in_struct(choice_target_cell);

  return dst;
}

/* Takes ownership of nr_cgi's buffer. */
static
rc_ctrl_req_data_t gen_handover_ctrl(byte_array_t nr_cgi)
{
  rc_ctrl_req_data_t dst = {0};

  // CONTROL HEADER, 9.2.2.11
  dst.hdr.format = FORMAT_1_E2SM_RC_CTRL_HDR;
  dst.hdr.frmt_1.ric_style_type = RC_CTRL_STYLE_CONN_MODE_MOBILITY;
  dst.hdr.frmt_1.ctrl_act_id = HANDOVER_CONTROL_7_6_4_1;
  dst.hdr.frmt_1.ue_id = ue_cell.ue_id;

  // CONTROL MESSAGE, 9.2.2.12
  dst.msg.format = FORMAT_1_E2SM_RC_CTRL_MSG;
  dst.msg.frmt_1.sz_ran_param = 1;
  dst.msg.frmt_1.ran_param = calloc(1, sizeof(seq_ran_param_t));
  assert(dst.msg.frmt_1.ran_param != NULL && "Memory exhausted");
  dst.msg.frmt_1.ran_param[0] = fill_target_primary_cell_id(nr_cgi);

  return dst;
}

/* Index of a RAN function in an E2 node's list, or sz when it is absent. The
 * xApp skips such nodes rather than failing, so no assert here. */
static
size_t find_ran_func_idx(const sm_ran_function_t* rf, size_t sz, int id)
{
  for (size_t i = 0; i < sz; i++)
    if (rf[i].id == id)
      return i;

  return sz;
}

/* Whether this E2 node advertises Connected mode mobility control with the
 * Handover Control action, i.e. whether sending the CONTROL is meaningful. */
static
bool supports_handover_ctrl(const ran_func_def_ctrl_t* ctrl)
{
  if (ctrl == NULL)
    return false;

  for (size_t i = 0; i < ctrl->sz_seq_ctrl_style; i++) {
    const seq_ctrl_style_t* style = &ctrl->seq_ctrl_style[i];
    if (style->style_type != RC_CTRL_STYLE_CONN_MODE_MOBILITY)
      continue;

    for (size_t j = 0; j < style->sz_seq_ctrl_act; j++)
      if (style->seq_ctrl_act[j].id == HANDOVER_CONTROL_7_6_4_1)
        return true;
  }

  return false;
}

/* Whether this E2 node advertises REPORT Style 5 (On Demand Report) with the
 * UE Context Information RAN parameter, i.e. whether subscribing with
 * gen_rc_sub_style_5() is meaningful. */
static
bool supports_on_demand_report(const ran_func_def_report_t* report)
{
  if (report == NULL)
    return false;

  for (size_t i = 0; i < report->sz_seq_report_sty; i++) {
    const seq_report_sty_t* style = &report->seq_report_sty[i];
    if (style->report_type != RC_REPORT_STYLE_ON_DEMAND)
      continue;

    for (size_t j = 0; j < style->sz_seq_ran_param; j++)
      if (style->ran_param[j].id == E2SM_RC_RS5_UE_CONTEXT_INFORMATION)
        return true;
  }

  return false;
}

/* NR CGI ::= SEQUENCE { pLMN-Identity, nRCellIdentity }, TS 38.473 9.3.1.7:
 * a 3-octet PLMN Identity followed by the 36-bit NR Cell Identity BIT
 * STRING, left-aligned in 5 octets (4 unused low bits in the last octet). */
static
byte_array_t decode_nr_cgi(const F1AP_NRCGI_t* nrcgi)
{
  assert(nrcgi->pLMN_Identity.size == 3 && "Unexpected PLMN Identity encoding");
  assert(nrcgi->nRCellIdentity.size == 5 && nrcgi->nRCellIdentity.bits_unused == 4 && "Unexpected NRCellIdentity encoding");

  byte_array_t dst = {.len = nrcgi->pLMN_Identity.size + nrcgi->nRCellIdentity.size};
  dst.buf = malloc(dst.len);
  assert(dst.buf != NULL && "Memory exhausted");
  memcpy(dst.buf, nrcgi->pLMN_Identity.buf, 3);
  memcpy(dst.buf + 3, nrcgi->nRCellIdentity.buf, 5);

  return dst;
}

/* Decode an F1 Setup Request (the raw APER-encoded F1AP PDU carried in an F1
 * E2 Node Component Config Addition item, E2AP 9.2.27) into its gNB-DU ID
 * and the NR CGI of its served cell. Returns a zeroed f1_du_t (nr_cgi.buf ==
 * NULL) if f1_setup_req is empty, does not decode, or declares no cell. */
static
f1_du_t decode_f1_setup_req(byte_array_t f1_setup_req)
{
  f1_du_t du = {0};

  if (f1_setup_req.buf == NULL || f1_setup_req.len == 0)
    return du;

  asn_codec_ctx_t st = {.max_stack_size = 100 * 1000};
  F1AP_F1AP_PDU_t* pdu = NULL;
  const asn_dec_rval_t rval = aper_decode(&st, &asn_DEF_F1AP_F1AP_PDU, (void**)&pdu, f1_setup_req.buf, f1_setup_req.len, 0, 0);
  if (rval.code != RC_OK) {
    fprintf(stderr, "Failed to decode F1 Setup Request\n");
    return du;
  }

  if (pdu->present != F1AP_F1AP_PDU_PR_initiatingMessage
      || pdu->choice.initiatingMessage->procedureCode != F1AP_ProcedureCode_id_F1Setup
      || pdu->choice.initiatingMessage->value.present != F1AP_InitiatingMessage__value_PR_F1SetupRequest) {
    fprintf(stderr, "E2 Node Component Config Addition (F1) does not carry an F1 Setup Request\n");
    ASN_STRUCT_FREE(asn_DEF_F1AP_F1AP_PDU, pdu);
    return du;
  }

  const F1AP_F1SetupRequest_t* req = &pdu->choice.initiatingMessage->value.choice.F1SetupRequest;
  for (int i = 0; i < req->protocolIEs.list.count; i++) {
    const F1AP_F1SetupRequestIEs_t* ie = req->protocolIEs.list.array[i];
    switch (ie->id) {
      case F1AP_ProtocolIE_ID_id_gNB_DU_ID: {
        unsigned long gnb_du_id = 0;
        asn_INTEGER2ulong(&ie->value.choice.GNB_DU_ID, &gnb_du_id);
        du.gnb_du_id = gnb_du_id;
        break;
      }
      case F1AP_ProtocolIE_ID_id_gNB_DU_Served_Cells_List: {
        /* Optional IE, 0 served cells if absent; this xApp only ever sees a
         * single-cell DU, so only the first item (if any) is kept. */
        const F1AP_GNB_DU_Served_Cells_List_t* cells = &ie->value.choice.GNB_DU_Served_Cells_List;
        if (cells->list.count > 0) {
          const F1AP_GNB_DU_Served_Cells_ItemIEs_t* item_ie = (const F1AP_GNB_DU_Served_Cells_ItemIEs_t*)cells->list.array[0];
          const F1AP_GNB_DU_Served_Cells_Item_t* cell = &item_ie->value.choice.GNB_DU_Served_Cells_Item;
          du.nr_cgi = decode_nr_cgi(&cell->served_Cell_Information.nRCGI);
        }
        break;
      }
      default:
        break;
    }
  }

  ASN_STRUCT_FREE(asn_DEF_F1AP_F1AP_PDU, pdu);

  if (du.nr_cgi.buf != NULL)
    printf("Decoded F1 Setup Request: gNB-DU ID %" PRIu64 ", NR Cell Identity %" PRIu64 "\n", du.gnb_du_id, nr_cgi_cell_id(&du.nr_cgi));
  else
    printf("Decoded F1 Setup Request: gNB-DU ID %" PRIu64 ", no served cell\n", du.gnb_du_id);

  return du;
}

static
param_report_def_t fill_param_report(uint32_t const ran_param_id, ran_param_def_t const* ran_param_def)
{
  param_report_def_t param_report = {0};

  param_report.ran_param_id = ran_param_id;
  if (ran_param_def != NULL) {
    param_report.ran_param_def = calloc(1, sizeof(ran_param_def_t));
    assert(param_report.ran_param_def != NULL && "Memory exhausted");
    *param_report.ran_param_def = cp_ran_param_def(ran_param_def);
  }

  return param_report;
}

/* REPORT Service Style 5 ("On Demand Report", E2SM-RC v01.03 7.4.6): Event
 * Trigger Format 5 (on demand, no per-UE/cell scoping). */
static
rc_sub_data_t gen_rc_sub_style_5(void)
{
  rc_sub_data_t rc_sub = {0};

  // Generate Event Trigger
  rc_sub.et.format = FORMAT_5_E2SM_RC_EV_TRIGGER_FORMAT;
  rc_sub.et.frmt_5.on_demand = TRUE_ON_DEMAND_FRMT_5;
  rc_sub.et.frmt_5.assoc_ue_info = NULL;
  rc_sub.et.frmt_5.assoc_cell_info = NULL;

  // Generate Action Definition
  rc_sub.sz_ad = 1;
  rc_sub.ad = calloc(rc_sub.sz_ad, sizeof(e2sm_rc_action_def_t));
  assert(rc_sub.ad != NULL && "Memory exhausted");
  rc_sub.ad[0].ric_style_type = RC_REPORT_STYLE_ON_DEMAND;
  rc_sub.ad[0].format = FORMAT_1_E2SM_RC_ACT_DEF;
  rc_sub.ad[0].frmt_1.sz_param_report_def = 1;
  rc_sub.ad[0].frmt_1.param_report_def = calloc(1, sizeof(param_report_def_t));
  assert(rc_sub.ad[0].frmt_1.param_report_def != NULL && "Memory exhausted");
  rc_sub.ad[0].frmt_1.param_report_def[0] = fill_param_report(E2SM_RC_RS5_UE_CONTEXT_INFORMATION, NULL);

  return rc_sub;
}

/* Posted once per received On Demand Report indication, so main() can block
 * until it arrives for a given node's subscription. */
static sem_t rc_ind_sem;

static
void sm_cb_rc(sm_ag_if_rd_t const* rd)
{
  assert(rd != NULL);
  assert(rd->type == INDICATION_MSG_AGENT_IF_ANS_V0);
  assert(rd->ind.rc.ind.msg.format == FORMAT_4_E2SM_RC_IND_MSG && "Expected Indication Message Format 4");

  const e2sm_rc_ind_msg_frmt_4_t* msg = &rd->ind.rc.ind.msg.frmt_4;
  assert(msg->sz_seq_ue_info == 1 && "Expected one UE connected.");

  const seq_ue_info_t* ue_info = &msg->seq_ue_info[0];
  ue_cell.ue_id = cp_ue_id_e2sm(&ue_info->ue_id);

  ue_cell.cell_global_id = cp_nr_cgi(&ue_info->cell_global_id.nr_cgi);

  const nr_cgi_t* nr_cgi = &ue_cell.cell_global_id;
  printf("UE currently on NR CGI: MCC %u, MNC %u (digit length %u), NR Cell Identity %" PRIu64 "\n",
         nr_cgi->plmn_id.mcc, nr_cgi->plmn_id.mnc, nr_cgi->plmn_id.mnc_digit_len, (uint64_t)nr_cgi->nr_cell_id);

  sem_post(&rc_ind_sem);
}

int main(int argc, char* argv[])
{
  fr_args_t args = init_fr_args(argc, argv);

  // Init the xApp
  init_xapp_api(&args);
  sleep(1);

  e2_node_arr_xapp_t nodes = e2_nodes_xapp_api();
  defer({ free_e2_node_arr_xapp(&nodes); });

  if (nodes.len == 0) {
    fprintf(stderr, "No E2 node connected\n");
    return EXIT_FAILURE;
  }

  /* Decode the F1 Setup Request(s) an E2 node registered as E2 Node Component
   * Config Addition items (E2AP 9.2.27) at E2 Setup, one per F1 connection. */
  f1_du_t* dus = NULL;
  size_t dus_len = 0;
  defer({ free_f1_dus(dus, dus_len); });
  for (int i = 0; i < nodes.len; i++) {
    const e2_node_connected_xapp_t* n = &nodes.n[i];
    for (int j = 0; j < n->len_cca; j++)
      if (n->id.type == ngran_gNB_DU && n->cca[j].e2_node_comp_interface_type == F1_E2AP_NODE_COMP_INTERFACE_TYPE) {
        dus = realloc(dus, (dus_len + 1) * sizeof(f1_du_t));
        assert(dus != NULL && "Memory exhausted");
        dus[dus_len++] = decode_f1_setup_req(n->cca[j].e2_node_comp_conf.request);
      }
  }

  /* Only the served cell of the first decoded gNB-DU is used as the
   * handover target. */
  if (dus_len == 0 || dus[0].nr_cgi.buf == NULL) {
    fprintf(stderr, "No NR CGI decoded from a connected gNB-DU's F1 Setup Request\n");
    return EXIT_FAILURE;
  }

  sem_init(&rc_ind_sem, 0, 0);
  defer({ sem_destroy(&rc_ind_sem); });

  /* The handover is driven at the CU-CP, which owns the UE context and the F1
   * connections to the candidate DUs, so only gNB-CU nodes are addressed. */
  for (int i = 0; i < nodes.len; i++) {
    e2_node_connected_xapp_t* n = &nodes.n[i];
    if (n->id.type != ngran_gNB_CU && n->id.type != ngran_gNB_CUCP)
      continue;

    const size_t idx = find_ran_func_idx(n->rf, n->len_rf, SM_RC_ID);
    if (idx == n->len_rf
        || supports_handover_ctrl(n->rf[idx].defn.rc.ctrl) == false
        || supports_on_demand_report(n->rf[idx].defn.rc.report) == false)
      continue;

    /* E2SM-RC Report Style 5 - On Demand Report, to get the currently
     * connected UE's ID and its serving NR CGI, torn down as soon as its one
     * indication is captured. */
    rc_sub_data_t rc_sub = gen_rc_sub_style_5();
    const sm_ans_xapp_t rc_sub_ans = report_sm_xapp_api(&n->id, SM_RC_ID, &rc_sub, sm_cb_rc);
    free_rc_sub_data(&rc_sub);

    if (rc_sub_ans.success == true) {
      sem_wait(&rc_ind_sem); // block for this node's one On Demand Report indication
      rm_report_sm_xapp_api(rc_sub_ans.u.handle);
    }

    const byte_array_t* target_nr_cgi = NULL;
    for (size_t i = 0; i < dus_len; i++) {
      const nr_cgi_t du_cgi = decode_nr_cgi_plmn_cell(&dus[i].nr_cgi);
      if (!eq_nr_cgi(&du_cgi, &ue_cell.cell_global_id)) {
        target_nr_cgi = &dus[i].nr_cgi;
        break;
      }
    }
    if (target_nr_cgi == NULL) {
      printf("No available cell for triggering handover on this E2 node\n");
      continue;
    }
    printf("Triggering UE handover to first available cell %" PRIu64 "\n", nr_cgi_cell_id(target_nr_cgi));

    /* E2SM-RC Control Style 3 - to send Handover Control Message for saved
     * UE ID and NR CGI different than the one currently connected to */
    rc_ctrl_req_data_t ctrl = gen_handover_ctrl(copy_byte_array(*target_nr_cgi));
    defer({ free_rc_ctrl_req_data(&ctrl); });

    const sm_ans_xapp_t ans = control_sm_xapp_api(&n->id, SM_RC_ID, &ctrl);
    if (ans.success == false) {
      fprintf(stderr, "E2 node rejected the handover CONTROL\n");
      return EXIT_FAILURE;
    }
  }

  while (try_stop_xapp_api() == false)
    usleep(1000);

  return EXIT_SUCCESS;
}
