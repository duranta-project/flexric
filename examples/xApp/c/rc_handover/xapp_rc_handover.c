/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include "../../../../src/xApp/e42_xapp_api.h"
#include "../../../../src/sm/rc_sm/ie/rc_data_ie.h"
#include "../../../../src/sm/rc_sm/rc_sm_id.h"
#include "../../../../src/sm/rc_sm/ie/ir/ran_param_struct.h"
#include "../../../../src/util/alg_ds/alg/defer.h"

#include "aper_decoder.h"
#include "F1AP_F1AP-PDU.h"
#include "F1AP_InitiatingMessage.h"
#include "F1AP_ProtocolIE-Field.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// E2SM-RC 7.6.4: Connected mode mobility control, Handover Control.
static const uint32_t RC_CTRL_STYLE_CONN_MODE_MOBILITY = 3;

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
rc_ctrl_req_data_t gen_handover_ctrl(uint32_t ue_id, byte_array_t nr_cgi)
{
  rc_ctrl_req_data_t dst = {0};

  // CONTROL HEADER, 9.2.2.11
  dst.hdr.format = FORMAT_1_E2SM_RC_CTRL_HDR;
  dst.hdr.frmt_1.ric_style_type = RC_CTRL_STYLE_CONN_MODE_MOBILITY;
  dst.hdr.frmt_1.ctrl_act_id = HANDOVER_CONTROL_7_6_4_1;
  dst.hdr.frmt_1.ue_id.type = GNB_DU_UE_ID_E2SM;
  dst.hdr.frmt_1.ue_id.gnb_du.gnb_cu_ue_f1ap = ue_id;

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
        // At the moment, assuming one DU only serves one cell.
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
void usage(const char* argv0)
{
  fprintf(stderr,
          "Usage: %s <ue_id> [-- <flexric flags>]\n"
          "  ue_id gNB-CU UE F1AP ID of the UE to hand over\n"
          "\n"
          "The target NR CGI is the first served cell of the first gNB-DU decoded out\n"
          "of a connected E2 node's F1 Setup Request(s).\n"
          "\n"
          "Everything after -- is passed to the xApp framework, e.g. -c <conf> -p <path>.\n",
          argv0);
}

int main(int argc, char* argv[])
{
  /* Positional arguments up to "--"; the rest belongs to the xApp framework,
   * whose own parser rejects anything it does not recognise. */
  int pos_argc = argc;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--") == 0) {
      pos_argc = i;
      break;
    }
  }

  if (pos_argc != 2) {
    usage(argv[0]);
    return EXIT_FAILURE;
  }

  const uint32_t ue_id = (uint32_t)strtoul(argv[1], NULL, 0);

  /* Re-present the framework flags as argv[0] followed by whatever came after
   * "--", which is the shape init_fr_args() expects. */
  int fr_argc = argc - pos_argc;
  char* fr_argv[argc];
  fr_argv[0] = argv[0];
  for (int i = 1; i < fr_argc; i++)
    fr_argv[i] = argv[pos_argc + i];

  fr_args_t args = init_fr_args(fr_argc > 0 ? fr_argc : 1, fr_argv);
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
  const byte_array_t* target_nr_cgi = &dus[0].nr_cgi;

  /* The handover is driven at the CU-CP, which owns the UE context and the F1
   * connections to the candidate DUs, so only gNB-CU nodes are addressed. */
  size_t sent = 0;
  for (int i = 0; i < nodes.len; i++) {
    e2_node_connected_xapp_t* n = &nodes.n[i];
    if (n->id.type != ngran_gNB_CU && n->id.type != ngran_gNB_CUCP)
      continue;

    const size_t idx = find_ran_func_idx(n->rf, n->len_rf, SM_RC_ID);
    if (idx == n->len_rf || supports_handover_ctrl(n->rf[idx].defn.rc.ctrl) == false)
      continue;

    rc_ctrl_req_data_t ctrl = gen_handover_ctrl(ue_id, copy_byte_array(*target_nr_cgi));
    defer({ free_rc_ctrl_req_data(&ctrl); });

    printf("Handover UE %" PRIu32 " to NR Cell Identity %" PRIu64 "\n", ue_id, nr_cgi_cell_id(target_nr_cgi));
    const sm_ans_xapp_t ans = control_sm_xapp_api(&n->id, SM_RC_ID, &ctrl);
    if (ans.success == false) {
      fprintf(stderr, "E2 node rejected the handover CONTROL\n");
      return EXIT_FAILURE;
    }
    sent++;
  }

  while (try_stop_xapp_api() == false)
    usleep(1000);

  if (sent == 0) {
    fprintf(stderr, "No connected gNB-CU advertises Handover Control (style %" PRIu32 ", action %" PRIu32 ")\n",
            RC_CTRL_STYLE_CONN_MODE_MOBILITY,
            (uint32_t)HANDOVER_CONTROL_7_6_4_1);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
