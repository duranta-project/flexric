/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include "../../../../src/xApp/e42_xapp_api.h"
#include "../../../../src/sm/rc_sm/ie/rc_data_ie.h"
#include "../../../../src/sm/rc_sm/rc_sm_id.h"
#include "../../../../src/sm/rc_sm/ie/ir/ran_param_struct.h"
#include "../../../../src/util/alg_ds/alg/defer.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// E2SM-RC 7.6.4: Connected mode mobility control, Handover Control.
static const uint32_t RC_CTRL_STYLE_CONN_MODE_MOBILITY = 3;

// NR CGI, TS 38.413 9.3.1.7: 3 octets of PLMN identity followed by a 36-bit
// NR Cell Identity left-aligned in 5 octets.
#define NR_CGI_LEN 8

static
byte_array_t enc_nr_cgi(const uint8_t plmn[3], uint64_t nr_cell_id)
{
  byte_array_t dst = {.len = NR_CGI_LEN};
  dst.buf = calloc(dst.len, sizeof(uint8_t));
  assert(dst.buf != NULL && "Memory exhausted");

  memcpy(dst.buf, plmn, 3);

  const uint64_t cell_id = nr_cell_id << 4; // 36 bits, left-aligned in 40
  for (size_t i = 0; i < 5; i++)
    dst.buf[3 + i] = (cell_id >> (8 * (4 - i))) & 0xFF;

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
 * nested shape 8.4.4.1 prescribes for Handover Control. */
static
seq_ran_param_t fill_target_primary_cell_id(const uint8_t plmn[3], uint64_t nr_cell_id)
{
  seq_ran_param_t nr_cgi = {.ran_param_id = NR_CGI_8_4_4_1};
  nr_cgi.ran_param_val.type = ELEMENT_KEY_FLAG_FALSE_RAN_PARAMETER_VAL_TYPE;
  nr_cgi.ran_param_val.flag_false = calloc(1, sizeof(ran_parameter_value_t));
  assert(nr_cgi.ran_param_val.flag_false != NULL && "Memory exhausted");
  nr_cgi.ran_param_val.flag_false->type = OCTET_STRING_RAN_PARAMETER_VALUE;
  nr_cgi.ran_param_val.flag_false->octet_str_ran = enc_nr_cgi(plmn, nr_cell_id);

  seq_ran_param_t nr_cell_id_p = {.ran_param_id = NR_CELL_8_4_4_1};
  nr_cell_id_p.ran_param_val = wrap_in_struct(nr_cgi);

  seq_ran_param_t choice_target_cell = {.ran_param_id = CHOICE_TARGET_CELL_8_4_4_1};
  choice_target_cell.ran_param_val = wrap_in_struct(nr_cell_id_p);

  seq_ran_param_t dst = {.ran_param_id = TARGET_PRIMARY_CELL_ID_8_4_4_1};
  dst.ran_param_val = wrap_in_struct(choice_target_cell);

  return dst;
}

static
rc_ctrl_req_data_t gen_handover_ctrl(uint32_t ue_id, const uint8_t plmn[3], uint64_t nr_cell_id)
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
  dst.msg.frmt_1.ran_param[0] = fill_target_primary_cell_id(plmn, nr_cell_id);

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

static
bool parse_plmn(const char* str, uint8_t plmn[3])
{
  unsigned b[3] = {0};
  if (sscanf(str, "%2x%2x%2x", &b[0], &b[1], &b[2]) != 3)
    return false;

  for (size_t i = 0; i < 3; i++)
    plmn[i] = (uint8_t)b[i];

  return true;
}

static
void usage(const char* argv0)
{
  fprintf(stderr,
          "Usage: %s <ue_id> <target_nr_cell_id> [plmn_hex] [-- <flexric flags>]\n"
          "  ue_id             gNB-CU UE F1AP ID of the UE to hand over\n"
          "  target_nr_cell_id 36-bit NR Cell Identity of the target cell\n"
          "  plmn_hex          6 hex digits of the target PLMN identity (default 000000)\n"
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

  if (pos_argc < 3 || pos_argc > 4) {
    usage(argv[0]);
    return EXIT_FAILURE;
  }

  const uint32_t ue_id = (uint32_t)strtoul(argv[1], NULL, 0);
  const uint64_t nr_cell_id = strtoull(argv[2], NULL, 0);

  uint8_t plmn[3] = {0, 0, 0};
  if (pos_argc == 4 && parse_plmn(argv[3], plmn) == false) {
    fprintf(stderr, "Could not parse '%s' as a 6-hex-digit PLMN identity\n", argv[3]);
    return EXIT_FAILURE;
  }

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

    rc_ctrl_req_data_t ctrl = gen_handover_ctrl(ue_id, plmn, nr_cell_id);
    defer({ free_rc_ctrl_req_data(&ctrl); });

    printf("Handover UE %" PRIu32 " to NR Cell Identity %" PRIu64 "\n", ue_id, nr_cell_id);
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
