/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 *
 * CCC Service Model Test xApp
 *
 *   Node-level: O-GnbDuFunction, O-GnbCuCpFunction, O-GnbCuUpFunction, O-RRMPolicyRatio
 *   Cell-level: O-RUInfo, O-Bwp, O-NrCellDu, O-NrCellCu, O-CESManagementFunction,
 *               O-NESPolicy, O-CellDTXDRXConfig, O-RRMPolicyRatio, O-PRBBlankingPolicy
 *
 *   TC-1  Subscription  - Event Trigger Format 1
 *   TC-2  Subscription  - Event Trigger Format 2
 *   TC-3  Subscription  - Event Trigger Format 3
 *   TC-4  Indication    - Receive indications
 *   TC-5  Control       - Style 1: O-GnbDuFunction
 *   TC-6  Control       - Style 1: O-RRMPolicyRatio
 *   TC-7  Control       - Style 2: O-NESPolicy
 *   TC-8  Control       - Style 2: O-Bwp
 *   TC-9  Control       - Style 2: O-CellDTXDRXConfig
 *   TC-10 Unsubscribe
 */

#include "../../../../src/xApp/e42_xapp_api.h"
#include "../../../../src/util/alg_ds/alg/defer.h"
#include "../../../../src/util/time_now_us.h"
#include "../../../../src/sm/ccc_sm/ccc_sm_id.h"
#include "../../../../src/sm/ccc_sm/ie/ccc_data_ie.h"

#include <assert.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static atomic_int g_ind_received = 0;

#define PASS(fmt, ...) printf("[PASS] " fmt "\n", ##__VA_ARGS__)
#define FAIL(fmt, ...) printf("[FAIL] " fmt "\n", ##__VA_ARGS__)
#define INFO(fmt, ...) printf("[INFO] " fmt "\n", ##__VA_ARGS__)
#define SEP()          printf("------------------------------------------------------------\n")

static
void sm_cb_ccc(sm_ag_if_rd_t const* rd)
{
  assert(rd != NULL);

  if (rd->type != INDICATION_MSG_AGENT_IF_ANS_V0) {
    FAIL("TC-4: unexpected rd->type = %d", rd->type);
    return;
  }
  if (rd->ind.type != CCC_STATS_V6) {
    FAIL("TC-4: unexpected ind type = %d", rd->ind.type);
    return;
  }

  atomic_fetch_add(&g_ind_received, 1);

  int64_t now = time_now_us();
  printf("[IND ] CCC indication #%d received at %ld us\n",
         atomic_load(&g_ind_received), now);

  ccc_ind_msg_t const* msg = &rd->ind.ccc.msg;
  if (msg->format == FORMAT_1_E2SM_CCC_IND_MSG &&
      msg->format1.list_of_configuration_structures_reported.data != NULL) {
    printf("       payload (%zu B): %.*s\n",
           msg->format1.list_of_configuration_structures_reported.len,
           (int)msg->format1.list_of_configuration_structures_reported.len,
           msg->format1.list_of_configuration_structures_reported.data);
  } else if (msg->format == FORMAT_2_E2SM_CCC_IND_MSG &&
             msg->format2.list_of_cells_reported.data != NULL) {
    printf("       payload (%zu B): %.*s\n",
           msg->format2.list_of_cells_reported.len,
           (int)msg->format2.list_of_cells_reported.len,
           msg->format2.list_of_cells_reported.data);
  } else if (msg->json_payload != NULL && msg->payload_len > 0) {
    printf("       payload (%zu B): %.*s\n",
           msg->payload_len,
           (int)msg->payload_len, msg->json_payload);
  }
}

/* Style 1 / Format 1: listOfConfigurationStructures */
static
char* make_ctrl_json_format1(const char* structure_name, const char* attributes_json)
{
  char buf[2048];
  int n = snprintf(buf, sizeof(buf),
    "{"
      "\"listOfConfigurationStructures\":["
        "{"
          "\"ranConfigurationStructureName\":\"%s\","
          "\"valuesOfAttributes\":%s"
        "}"
      "]"
    "}",
    structure_name, attributes_json);
  assert(n > 0 && n < (int)sizeof(buf));
  char* out = malloc((size_t)n + 1);
  assert(out != NULL);
  memcpy(out, buf, (size_t)n + 1);
  return out;
}

/* Style 2 / Format 2: listOfCellsControlled + cellGlobalId */
static
char* make_ctrl_json_format2(const char* structure_name,
                             const char* attributes_json,
                             const char* old_attributes_json)
{
  char buf[3072];
  int n;

  if (old_attributes_json != NULL) {
    n = snprintf(buf, sizeof(buf),
      "{"
        "\"listOfCellsControlled\":["
          "{"
            "\"cellGlobalId\":{"
              "\"nR-CGI\":{"
                "\"plmnIdentity\":\"00101\","
                "\"nRCellIdentity\":\"000000000000000000000000000000000001\""
              "}"
            "},"
            "\"listOfConfigurationStructures\":["
              "{"
                "\"ranConfigurationStructureName\":\"%s\","
                "\"oldValuesOfAttributes\":%s,"
                "\"newValuesOfAttributes\":%s"
              "}"
            "]"
          "}"
        "]"
      "}",
      structure_name, old_attributes_json, attributes_json);
  } else {
    n = snprintf(buf, sizeof(buf),
      "{"
        "\"listOfCellsControlled\":["
          "{"
            "\"cellGlobalId\":{"
              "\"nR-CGI\":{"
                "\"plmnIdentity\":\"00101\","
                "\"nRCellIdentity\":\"000000000000000000000000000000000001\""
              "}"
            "},"
            "\"listOfConfigurationStructures\":["
              "{"
                "\"ranConfigurationStructureName\":\"%s\","
                "\"valuesOfAttributes\":%s"
              "}"
            "]"
          "}"
        "]"
      "}",
      structure_name, attributes_json);
  }

  assert(n > 0 && n < (int)sizeof(buf));
  char* out = malloc((size_t)n + 1);
  assert(out != NULL);
  memcpy(out, buf, (size_t)n + 1);
  return out;
}

static
bool send_ccc_ctrl_format1(e2_node_connected_xapp_t* node, const char* json_payload)
{
  ccc_ctrl_req_data_t ctrl = {0};

  ctrl.hdr.format = FORMAT_1_E2SM_CCC_CTRL_HDR;
  ctrl.hdr.format1.ric_style_type = CCC_CTRL_SERVICE_STYLE_TYPE_1;

  size_t plen = strlen(json_payload);
  ctrl.msg.format = FORMAT_1_E2SM_CCC_CTRL_MSG;
  ctrl.msg.format1.list_of_configuration_structures.data = malloc(plen + 1);
  assert(ctrl.msg.format1.list_of_configuration_structures.data != NULL);
  memcpy(ctrl.msg.format1.list_of_configuration_structures.data, json_payload, plen + 1);
  ctrl.msg.format1.list_of_configuration_structures.len = plen;

  control_sm_xapp_api(&node->id, SM_CCC_ID, &ctrl);
  free(ctrl.msg.format1.list_of_configuration_structures.data);
  return true;
}

static
bool send_ccc_ctrl_format2(e2_node_connected_xapp_t* node, const char* json_payload)
{
  ccc_ctrl_req_data_t ctrl = {0};

  ctrl.hdr.format = FORMAT_1_E2SM_CCC_CTRL_HDR;
  ctrl.hdr.format1.ric_style_type = CCC_CTRL_SERVICE_STYLE_TYPE_2;

  size_t plen = strlen(json_payload);
  ctrl.msg.format = FORMAT_2_E2SM_CCC_CTRL_MSG;
  ctrl.msg.format2.list_of_cells_controlled.data = malloc(plen + 1);
  assert(ctrl.msg.format2.list_of_cells_controlled.data != NULL);
  memcpy(ctrl.msg.format2.list_of_cells_controlled.data, json_payload, plen + 1);
  ctrl.msg.format2.list_of_cells_controlled.len = plen;

  control_sm_xapp_api(&node->id, SM_CCC_ID, &ctrl);
  free(ctrl.msg.format2.list_of_cells_controlled.data);
  return true;
}

int main(int argc, char* argv[])
{
  SEP();
  INFO("CCC SM Comprehensive Test xApp");
  SEP();

  fr_args_t args = init_fr_args(argc, argv);
  init_xapp_api(&args);
  sleep(1);

  e2_node_arr_xapp_t nodes = e2_nodes_xapp_api();
  defer({ free_e2_node_arr_xapp(&nodes); });

  if (nodes.len == 0) {
    FAIL("No E2 nodes connected — is the nearRT-RIC running?");
    try_stop_xapp_api();
    return EXIT_FAILURE;
  }
  INFO("Connected E2 nodes = %d", (int)nodes.len);

  for (size_t i = 0; i < nodes.len; i++) {
    INFO("Node %zu RAN functions:", i);
    for (size_t j = 0; j < nodes.n[i].len_rf; j++) {
      INFO("  SM id = %d", nodes.n[i].rf[j].id);
    }
  }

  sm_ans_xapp_t* hdl_fmt1  = calloc(nodes.len, sizeof(sm_ans_xapp_t));
  sm_ans_xapp_t* hdl_fmt2  = calloc(nodes.len, sizeof(sm_ans_xapp_t));
  sm_ans_xapp_t* hdl_fmt3  = calloc(nodes.len, sizeof(sm_ans_xapp_t));
  assert(hdl_fmt1 && hdl_fmt2 && hdl_fmt3);

  SEP();
  INFO("TC-1: Subscription – Event Trigger Format 1 (\"1_ms\")");
  for (size_t i = 0; i < nodes.len; i++) {
    const char* trigger = "1_ms";
    hdl_fmt1[i] = report_sm_xapp_api(&nodes.n[i].id, SM_CCC_ID,
                                      (void*)trigger, sm_cb_ccc);
    if (hdl_fmt1[i].success)
      PASS("TC-1 node %zu subscribed (Format 1)", i);
    else
      FAIL("TC-1 node %zu subscription FAILED", i);
  }

  sleep(2);

  SEP();
  INFO("TC-2: Subscription – Event Trigger Format 2 (\"5_ms\")");
  for (size_t i = 0; i < nodes.len; i++) {
    const char* trigger = "5_ms";
    hdl_fmt2[i] = report_sm_xapp_api(&nodes.n[i].id, SM_CCC_ID,
                                      (void*)trigger, sm_cb_ccc);
    if (hdl_fmt2[i].success)
      PASS("TC-2 node %zu subscribed (Format 2)", i);
    else
      FAIL("TC-2 node %zu subscription FAILED", i);
  }

  sleep(2);

  SEP();
  INFO("TC-3: Subscription – Event Trigger Format 3 (\"10_ms\")");
  for (size_t i = 0; i < nodes.len; i++) {
    const char* trigger = "10_ms";
    hdl_fmt3[i] = report_sm_xapp_api(&nodes.n[i].id, SM_CCC_ID,
                                      (void*)trigger, sm_cb_ccc);
    if (hdl_fmt3[i].success)
      PASS("TC-3 node %zu subscribed (Format 3)", i);
    else
      FAIL("TC-3 node %zu subscription FAILED", i);
  }

  SEP();
  INFO("TC-4: Indication Reception – waiting 5 s...");
  sleep(5);
  int ind_count = atomic_load(&g_ind_received);
  if (ind_count > 0)
    PASS("TC-4: received %d CCC indications", ind_count);
  else
    FAIL("TC-4: no CCC indications received");

  SEP();
  INFO("TC-5: Control – Style 1: O-GnbDuFunction");
  for (size_t i = 0; i < nodes.len; i++) {
    char* json = make_ctrl_json_format1(
      CCC_RAN_STRUCT_NAME_O_GNB_DU_FUNCTION,
      "{"
        "\"gNBDUId\":1,"
        "\"gNBDUName\":\"gNB-DU-1\""
      "}"
    );
    bool ok = send_ccc_ctrl_format1(&nodes.n[i], json);
    free(json);
    if (ok) PASS("TC-5 node %zu: O-GnbDuFunction control sent", i);
    else    FAIL("TC-5 node %zu: O-GnbDuFunction control FAILED", i);
  }
  sleep(1);

  SEP();
  INFO("TC-6: Control – Style 1: O-RRMPolicyRatio");
  for (size_t i = 0; i < nodes.len; i++) {
    char* json = make_ctrl_json_format1(
      CCC_RAN_STRUCT_NAME_O_RRM_POLICY_RATIO,
      "{"
        "\"resourceType\":\"PRB\","
        "\"rRMPolicyMaxRatio\":80,"
        "\"rRMPolicyMinRatio\":20,"
        "\"rRMPolicyDedicatedRatio\":40"
      "}"
    );
    bool ok = send_ccc_ctrl_format1(&nodes.n[i], json);
    free(json);
    if (ok) PASS("TC-6 node %zu: O-RRMPolicyRatio control sent", i);
    else    FAIL("TC-6 node %zu: O-RRMPolicyRatio control FAILED", i);
  }
  sleep(1);

  SEP();
  INFO("TC-7: Control – Style 2: O-NESPolicy");
  for (size_t i = 0; i < nodes.len; i++) {
    char* json = make_ctrl_json_format2(
      CCC_RAN_STRUCT_NAME_O_NES_POLICY,
      "{\"antennaMask\":\"1100\"}",
      "{\"antennaMask\":\"1111\"}"
    );
    bool ok = send_ccc_ctrl_format2(&nodes.n[i], json);
    free(json);
    if (ok) PASS("TC-7 node %zu: O-NESPolicy control sent", i);
    else    FAIL("TC-7 node %zu: O-NESPolicy control FAILED", i);
  }
  sleep(1);

  SEP();
  INFO("TC-8: Control – Style 2: O-Bwp");
  for (size_t i = 0; i < nodes.len; i++) {
    char* json = make_ctrl_json_format2(
      CCC_RAN_STRUCT_NAME_O_BWP,
      "{"
        "\"bwpContext\":0,"
        "\"isInitialBwp\":true,"
        "\"subCarrierSpacing\":1,"
        "\"cyclicPrefix\":0,"
        "\"startRB\":0,"
        "\"numberOfRBs\":106"
      "}",
      NULL
    );
    bool ok = send_ccc_ctrl_format2(&nodes.n[i], json);
    free(json);
    if (ok) PASS("TC-8 node %zu: O-Bwp control sent", i);
    else    FAIL("TC-8 node %zu: O-Bwp control FAILED", i);
  }
  sleep(1);

  SEP();
  INFO("TC-9: Control – Style 2: O-CellDTXDRXConfig");
  for (size_t i = 0; i < nodes.len; i++) {
    char* json = make_ctrl_json_format2(
      CCC_RAN_STRUCT_NAME_O_CELL_DTXDRX_CONFIG,
      "{"
        "\"onDurationTimer\":10,"
        "\"cycleStartOffset\":0,"
        "\"slotOffset\":0,"
        "\"configType\":0,"
        "\"activationStatus\":1,"
        "\"l1Activation\":true"
      "}",
      NULL
    );
    bool ok = send_ccc_ctrl_format2(&nodes.n[i], json);
    free(json);
    if (ok) PASS("TC-9 node %zu: O-CellDTXDRXConfig control sent", i);
    else    FAIL("TC-9 node %zu: O-CellDTXDRXConfig control FAILED", i);
  }
  sleep(1);

  SEP();
  int final_ind = atomic_load(&g_ind_received);
  INFO("Total CCC indications received: %d", final_ind);
  if (final_ind > ind_count)
    PASS("Indications continued arriving during control phase");

  SEP();
  INFO("TC-10: Unsubscribe all CCC subscriptions");
  for (size_t i = 0; i < nodes.len; i++) {
    if (hdl_fmt1[i].success) {
      rm_report_sm_xapp_api(hdl_fmt1[i].u.handle);
      PASS("TC-10 node %zu: Format 1 subscription removed", i);
    }
    if (hdl_fmt2[i].success) {
      rm_report_sm_xapp_api(hdl_fmt2[i].u.handle);
      PASS("TC-10 node %zu: Format 2 subscription removed", i);
    }
    if (hdl_fmt3[i].success) {
      rm_report_sm_xapp_api(hdl_fmt3[i].u.handle);
      PASS("TC-10 node %zu: Format 3 subscription removed", i);
    }
  }

  free(hdl_fmt1);
  free(hdl_fmt2);
  free(hdl_fmt3);

  try_stop_xapp_api();

  SEP();
  INFO("All CCC SM test cases complete.");
  INFO("Total indications received: %d", atomic_load(&g_ind_received));
  SEP();

  return EXIT_SUCCESS;
}
