/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 *
 * CCC Service Model unit test — encode/decode round-trip.
 *
 * Tests performed (no RIC, no ns-3, no network required):
 *   1. check_eq_ran_function  – agent and RIC share the same SM ID (4)
 *   2. check_subscription     – event trigger Format 1 period encoded by RIC,
 *                               decoded by agent as PERIODIC_SUBSCRIPTION_FLRC
 *   3. check_indication       – agent encodes indication (Format 1, camelCase
 *                               JSON keys per e2sm_ccc.hpp), RIC decodes it;
 *                               the JSON key "listOfConfigurationStructuresReported"
 *                               must survive the round-trip
 *   4. check_ctrl_f1          – RIC encodes a node-level (Format 1) control
 *                               message, agent decodes it; key
 *                               "listOfConfigurationStructures" must survive
 *   5. check_ctrl_f2          – RIC encodes a cell-level (Format 2) control
 *                               message, agent decodes it; key
 *                               "listOfCellsControlled" must survive
 */

#include "../../../src/sm/ccc_sm/ccc_sm_agent.h"
#include "../../../src/sm/ccc_sm/ccc_sm_ric.h"
#include "../../../src/sm/ccc_sm/ie/ccc_data_ie.h"
#include "../../../src/sm/ccc_sm/ccc_sm_id.h"
#include "../../../src/sm/sm_proc_data.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* ── agent I/O callback: fill a default Format-1 indication ─────────────── */

static
bool read_ind_ccc(void* read)
{
  assert(read != NULL);

  ccc_rd_ind_data_t* ccc = (ccc_rd_ind_data_t*)read;

  /* Indication header – Format 1 */
  ccc->ind.hdr.format = FORMAT_1_E2SM_CCC_IND_HDR;
  ccc->ind.hdr.format1.event_time        = 123456789ULL;
  ccc->ind.hdr.format1.indication_reason = NULL;

  /* Indication message – Format 1 with camelCase JSON payload */
  /* Indication message – Format 1 */
  static const char ind_payload[] =
    "{\"listOfConfigurationStructuresReported\":["
      "{\"ranConfigurationStructureName\":\"O-GnbDuFunction\","
       "\"valuesOfAttributes\":{"
         "\"gNBDUId\":1,"
         "\"gNBDUName\":\"gNB-DU-1\""
       "}"
      "}"
    "]}";

  size_t len = strlen(ind_payload);
  ccc->ind.msg.format = FORMAT_1_E2SM_CCC_IND_MSG;
  ccc->ind.msg.format1.list_of_configuration_structures_reported.data =
      (char*)malloc(len + 1);
  assert(ccc->ind.msg.format1.list_of_configuration_structures_reported.data
         != NULL);
  memcpy(ccc->ind.msg.format1.list_of_configuration_structures_reported.data,
         ind_payload, len + 1);
  ccc->ind.msg.format1.list_of_configuration_structures_reported.len = len;

  return true;
}

/* ── agent I/O callback: receive a control message ──────────────────────── */

/*
 * The agent frees ctrl_msg memory before on_control() returns, so any
 * pointer captured from the decoded struct would be dangling afterwards.
 * We make a heap copy here and free it in the test once done.
 */
static char* g_ctrl_payload_copy = NULL;

static
sm_ag_if_ans_t write_ctrl_ccc(void const* data)
{
  assert(data != NULL);
  ccc_ctrl_req_data_t const* req = (ccc_ctrl_req_data_t const*)data;

  const char* raw = NULL;
  if (req->msg.format == FORMAT_1_E2SM_CCC_CTRL_MSG)
    raw = req->msg.format1.list_of_configuration_structures.data;
  else if (req->msg.format == FORMAT_2_E2SM_CCC_CTRL_MSG)
    raw = req->msg.format2.list_of_cells_controlled.data;
  else if (req->msg.json_payload != NULL)
    raw = req->msg.json_payload;

  /* Copy before the agent frees it */
  if (g_ctrl_payload_copy) { free(g_ctrl_payload_copy); g_ctrl_payload_copy = NULL; }
  if (raw) g_ctrl_payload_copy = strdup(raw);

  sm_ag_if_ans_t ans = {.type = CTRL_OUTCOME_SM_AG_IF_ANS_V0};
  ans.ctrl_out.type  = CCC_AGENT_IF_CTRL_ANS_V0;
  ans.ctrl_out.ccc.outcome = 0;
  return ans;
}

/* ══════════════════════════════════════════════════════════════════════════
 * Test 1 – SM IDs match
 * ══════════════════════════════════════════════════════════════════════════ */
static
void check_eq_ran_function(sm_agent_t const* ag, sm_ric_t const* ric)
{
  assert(ag->info.id()   == SM_CCC_ID);
  assert(ric->ran_func_id == SM_CCC_ID);
  printf("[PASS] TC-1: agent SM ID == RIC SM ID == %u\n", SM_CCC_ID);
}

/* ══════════════════════════════════════════════════════════════════════════
 * Test 2 – Subscription encode/decode
 * ══════════════════════════════════════════════════════════════════════════ */
static
void check_subscription(sm_agent_t* ag, sm_ric_t* ric)
{
  char sub[] = "1_ms";
  sm_subs_data_t data = ric->proc.on_subscription(ric, &sub);

  sm_ag_if_ans_subs_t const subs = ag->proc.on_subscription(ag, &data);
  assert(subs.type  == PERIODIC_SUBSCRIPTION_FLRC);
  assert(subs.per.t.ms == 1);

  free_sm_subs_data(&data);
  printf("[PASS] TC-2: subscription encode/decode (period = 1 ms)\n");
}

/* ══════════════════════════════════════════════════════════════════════════
 * Test 3 – Indication encode (agent) → decode (RIC)
 * ══════════════════════════════════════════════════════════════════════════ */
static
void check_indication(sm_agent_t* ag, sm_ric_t* ric)
{
  /* Agent encodes */
  exp_ind_data_t exp = ag->proc.on_indication(ag, NULL);
  assert(exp.has_value == true);
  assert(exp.data.len_hdr > 0 && "Indication header bytes must be non-empty");
  assert(exp.data.len_msg > 0 && "Indication message bytes must be non-empty");

  /* RIC decodes */
  sm_ag_if_rd_ind_t msg = ric->proc.on_indication(ric, &exp.data);
  assert(msg.type == CCC_STATS_V6);

  /* Verify the camelCase key survived the round-trip */
  ccc_ind_data_t* ind = &msg.ccc;
  const char*     decoded_payload = NULL;

  if (ind->msg.format == FORMAT_1_E2SM_CCC_IND_MSG)
    decoded_payload =
        ind->msg.format1.list_of_configuration_structures_reported.data;
  else if (ind->msg.json_payload != NULL)
    decoded_payload = ind->msg.json_payload;

  assert(decoded_payload != NULL && "Decoded indication payload must not be NULL");
  assert(strstr(decoded_payload, "\"listOfConfigurationStructuresReported\"") != NULL
         && "camelCase key missing from decoded indication");
  assert(strstr(decoded_payload, "\"ranConfigurationStructureName\"") != NULL
         && "ranConfigurationStructureName missing from decoded indication");

  printf("[PASS] TC-3: indication round-trip – payload: %s\n", decoded_payload);

  /* Free decoded data */
  free_ccc_ind_hdr(&ind->hdr);
  free_ccc_ind_msg(&ind->msg);
  free_exp_ind_data(&exp);
}

/* ══════════════════════════════════════════════════════════════════════════
 * Test 4 – Control Format 1 encode (RIC) → decode (agent)
 *           Node-level: "listOfConfigurationStructures"
 * ══════════════════════════════════════════════════════════════════════════ */
static
void check_ctrl_f1(sm_agent_t* ag, sm_ric_t* ric)
{
  static const char f1_payload[] =
    "{\"listOfConfigurationStructures\":["
      "{\"ranConfigurationStructureName\":\"O-GnbDuFunction\","
       "\"valuesOfAttributes\":{"
         "\"gNBDUId\":1,"
         "\"gNBDUName\":\"gNB-DU-1\""
       "}"
      "}"
    "]}";

  ccc_ctrl_req_data_t ctrl = {0};
  ctrl.hdr.format = FORMAT_1_E2SM_CCC_CTRL_HDR;
  ctrl.hdr.format1.ric_style_type = CCC_CTRL_SERVICE_STYLE_TYPE_1;

  ctrl.msg.format = FORMAT_1_E2SM_CCC_CTRL_MSG;
  ctrl.msg.format1.list_of_configuration_structures.data = (char*)f1_payload;
  ctrl.msg.format1.list_of_configuration_structures.len  = strlen(f1_payload);

  /* RIC encodes */
  sm_ctrl_req_data_t req = ric->proc.on_control_req(ric, &ctrl);
  assert(req.len_hdr > 0 && req.len_msg > 0);

  /* Agent decodes and calls write_ctrl_ccc */
  g_ctrl_payload_copy = NULL;
  sm_ctrl_out_data_t out  = ag->proc.on_control(ag, &req);
  assert(out.ctrl_out != NULL && out.len_out > 0
         && "Agent must return a non-empty outcome");

  /* Verify that the agent saw the camelCase key (using safe copy) */
  assert(g_ctrl_payload_copy != NULL
         && "write_ctrl_ccc was not called or payload was NULL");
  assert(strstr(g_ctrl_payload_copy, "\"listOfConfigurationStructures\"") != NULL
         && "camelCase key missing in Format 1 control decoded by agent");

  /* Verify outcome uses camelCase keys */
  assert(strstr((char*)out.ctrl_out, "\"receivedTimestamp\"") != NULL
         && "camelCase key receivedTimestamp missing from control outcome");

  printf("[PASS] TC-4: ctrl Format-1 round-trip – agent decoded: %s\n",
         g_ctrl_payload_copy);

  free(g_ctrl_payload_copy); g_ctrl_payload_copy = NULL;
  free_sm_ctrl_req_data(&req);
  free(out.ctrl_out);
}

/* ══════════════════════════════════════════════════════════════════════════
 * Test 5 – Control Format 2 encode (RIC) → decode (agent)
 *           Cell-level: "listOfCellsControlled"
 * ══════════════════════════════════════════════════════════════════════════ */
static
void check_ctrl_f2(sm_agent_t* ag, sm_ric_t* ric)
{
  static const char f2_payload[] =
    "{\"listOfCellsControlled\":["
      "{\"cellGlobalId\":{"
        "\"nR-CGI\":{"
          "\"plmnIdentity\":\"00101\","
          "\"nRCellIdentity\":\"000000000000000000000000000000000001\""
        "}"
       "},"
       "\"listOfConfigurationStructures\":["
         "{\"ranConfigurationStructureName\":\"O-NESPolicy\","
          "\"oldValuesOfAttributes\":{\"antennaMask\":\"1111\"},"
          "\"newValuesOfAttributes\":{\"antennaMask\":\"1100\"}"
         "}"
       "]"
      "}"
    "]}";

  ccc_ctrl_req_data_t ctrl = {0};
  ctrl.hdr.format = FORMAT_1_E2SM_CCC_CTRL_HDR;
  ctrl.hdr.format1.ric_style_type = CCC_CTRL_SERVICE_STYLE_TYPE_2;

  ctrl.msg.format = FORMAT_2_E2SM_CCC_CTRL_MSG;
  ctrl.msg.format2.list_of_cells_controlled.data = (char*)f2_payload;
  ctrl.msg.format2.list_of_cells_controlled.len  = strlen(f2_payload);

  /* RIC encodes */
  sm_ctrl_req_data_t req = ric->proc.on_control_req(ric, &ctrl);
  assert(req.len_hdr > 0 && req.len_msg > 0);

  /* Agent decodes */
  g_ctrl_payload_copy = NULL;
  sm_ctrl_out_data_t out  = ag->proc.on_control(ag, &req);
  assert(out.ctrl_out != NULL && out.len_out > 0);

  assert(g_ctrl_payload_copy != NULL
         && "write_ctrl_ccc was not called or payload was NULL");
  assert(strstr(g_ctrl_payload_copy, "\"listOfCellsControlled\"") != NULL
         && "camelCase key missing in Format 2 control decoded by agent");
  assert(strstr(g_ctrl_payload_copy, "\"ranConfigurationStructureName\"") != NULL
         && "ranConfigurationStructureName missing in Format 2 ctrl payload");
  assert(strstr(g_ctrl_payload_copy, "\"oldValuesOfAttributes\"") != NULL
         && "oldValuesOfAttributes missing in Format 2 ctrl payload");
  assert(strstr(g_ctrl_payload_copy, "\"newValuesOfAttributes\"") != NULL
         && "newValuesOfAttributes missing in Format 2 ctrl payload");

  printf("[PASS] TC-5: ctrl Format-2 round-trip – agent decoded: %s\n",
         g_ctrl_payload_copy);

  free(g_ctrl_payload_copy); g_ctrl_payload_copy = NULL;
  free_sm_ctrl_req_data(&req);
  free(out.ctrl_out);
}

/* ══════════════════════════════════════════════════════════════════════════
 * main
 * ══════════════════════════════════════════════════════════════════════════ */
int main(void)
{
  sm_io_ag_ran_t io_ag = {0};
  io_ag.read_ind_tbl[CCC_STATS_V6]   = read_ind_ccc;
  io_ag.write_ctrl_tbl[CCC_CTRL_REQ_V0] = write_ctrl_ccc;

  sm_agent_t* sm_ag  = make_ccc_sm_agent(io_ag);
  sm_ric_t*   sm_ric = make_ccc_sm_ric();

  check_eq_ran_function(sm_ag, sm_ric);
  check_subscription   (sm_ag, sm_ric);
  check_indication     (sm_ag, sm_ric);
  check_ctrl_f1        (sm_ag, sm_ric);
  check_ctrl_f2        (sm_ag, sm_ric);

  sm_ag->free_sm(sm_ag);
  sm_ric->free_sm(sm_ric);

  printf("\nAll CCC SM unit tests passed.\n");
  return EXIT_SUCCESS;
}
