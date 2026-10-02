/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */
#include "sm_ccc.h"
#include "../../../src/sm/ccc_sm/ie/ccc_data_ie.h"
#include "../../../src/util/time_now_us.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void init_ccc_sm(void)
{
}

void free_ccc_sm(void)
{
}

bool read_ccc_sm(void* data)
{
  assert(data != NULL);

  ccc_rd_ind_data_t* ccc = (ccc_rd_ind_data_t*)data;

  ccc->ind.hdr.format = FORMAT_1_E2SM_CCC_IND_HDR;
  ccc->ind.hdr.format1.event_time = (uint64_t)time_now_us();
  ccc->ind.hdr.format1.indication_reason = NULL;

  static const char payload[] =
    "{"
      "\"listOfConfigurationStructuresReported\":["
        "{"
          "\"ranConfigurationStructureName\":\"O-GnbDuFunction\","
          "\"valuesOfAttributes\":{"
            "\"gNBDUId\":1,"
            "\"gNBDUName\":\"emu-gNB-DU-1\""
          "}"
        "}"
      "]"
    "}";

  size_t len = strlen(payload);
  ccc->ind.msg.format = FORMAT_1_E2SM_CCC_IND_MSG;
  ccc->ind.msg.format1.list_of_configuration_structures_reported.data =
      calloc(len + 1, sizeof(char));
  assert(ccc->ind.msg.format1.list_of_configuration_structures_reported.data != NULL
         && "Memory exhausted");
  memcpy(ccc->ind.msg.format1.list_of_configuration_structures_reported.data,
         payload, len + 1);
  ccc->ind.msg.format1.list_of_configuration_structures_reported.len = len;

  return true;
}

void read_ccc_setup_sm(void* data)
{
  assert(data != NULL);
  (void)data;
}

sm_ag_if_ans_t write_ctrl_ccc_sm(void const* data)
{
  assert(data != NULL);

  ccc_ctrl_req_data_t const* req = (ccc_ctrl_req_data_t const*)data;

  const char* json = NULL;
  size_t json_len = 0;

  if (req->msg.format == FORMAT_1_E2SM_CCC_CTRL_MSG) {
    json = req->msg.format1.list_of_configuration_structures.data;
    json_len = req->msg.format1.list_of_configuration_structures.len;
  } else if (req->msg.format == FORMAT_2_E2SM_CCC_CTRL_MSG) {
    json = req->msg.format2.list_of_cells_controlled.data;
    json_len = req->msg.format2.list_of_cells_controlled.len;
  } else if (req->msg.json_payload != NULL) {
    json = req->msg.json_payload;
    json_len = req->msg.payload_len;
  }

  if (json != NULL && json_len > 0) {
    printf("[E2 AGENT][CCC]: Control received (%zu B): %.*s\n",
           json_len, (int)json_len, json);
  } else {
    printf("[E2 AGENT][CCC]: Control received (empty payload)\n");
  }

  sm_ag_if_ans_t ans = {.type = CTRL_OUTCOME_SM_AG_IF_ANS_V0};
  ans.ctrl_out.type = CCC_AGENT_IF_CTRL_ANS_V0;
  ans.ctrl_out.ccc.outcome = 0;
  return ans;
}
