/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include "ccc_dec_json.h"
#include "../ccc_sm.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

ccc_event_trigger_t ccc_dec_event_trigger_json(size_t len, uint8_t const ev_tr[len])
{
  assert(len >= sizeof(ccc_event_trigger_t));
  assert(ev_tr != NULL);

  ccc_event_trigger_t ret = {0};
  memcpy(&ret, ev_tr, sizeof(ccc_event_trigger_t));
  return ret;
}

ccc_action_def_t ccc_dec_action_def_json(size_t len, uint8_t const action_def[len])
{
  assert(len >= sizeof(ccc_action_def_t));
  assert(action_def != NULL);

  ccc_action_def_t ret = {0};
  memcpy(&ret, action_def, sizeof(ccc_action_def_t));
  return ret;
}

ccc_ind_hdr_t ccc_dec_ind_hdr_json(size_t len, uint8_t const ind_hdr[len])
{
  assert(ind_hdr != NULL);

  ccc_ind_hdr_t ret = {0};

  /* Wire key "eventTime" per E2SmCccIndicationHeaderFormat1Properties. */
  char* json_str = ccc_sm_decode_ind_hdr(ind_hdr, len);
  if (json_str) {
    long long evt = 0;
    if (sscanf(json_str, "{\"eventTime\":%lld", &evt) == 1) {
      ret.format = FORMAT_1_E2SM_CCC_IND_HDR;
      ret.format1.event_time = (uint64_t)evt;
    }
    free(json_str);
  }
  return ret;
}

ccc_ind_msg_t ccc_dec_ind_msg_json(size_t len, uint8_t const ind_msg[len])
{
  assert(ind_msg != NULL);

  ccc_ind_msg_t ret = {0};

  if (len > 0) {
    char* json_str = ccc_sm_decode_ind_msg(ind_msg, len);
    if (json_str) {
      if (strstr(json_str, "\"listOfCellsReported\"") != NULL) {
        ret.format = FORMAT_2_E2SM_CCC_IND_MSG;
        size_t slen = strlen(json_str);
        ret.format2.list_of_cells_reported.data = json_str;
        ret.format2.list_of_cells_reported.len  = slen;
      } else {
        ret.format = FORMAT_1_E2SM_CCC_IND_MSG;
        size_t slen = strlen(json_str);
        ret.format1.list_of_configuration_structures_reported.data = json_str;
        ret.format1.list_of_configuration_structures_reported.len  = slen;
      }
    }
  }
  return ret;
}

ccc_ctrl_hdr_t ccc_dec_ctrl_hdr_json(size_t len, uint8_t const ctrl_hdr[len])
{
  assert(ctrl_hdr != NULL);

  ccc_ctrl_hdr_t ret = {0};

  /* Wire key "ricStyleType" per E2SmCccControlHeaderFormat1Properties. */
  char* json_str = ccc_sm_decode_ctrl_hdr(ctrl_hdr, len);
  if (json_str) {
    int style = 0;
    if (sscanf(json_str, "{\"ricStyleType\":%d", &style) == 1) {
      ret.format = FORMAT_1_E2SM_CCC_CTRL_HDR;
      ret.format1.ric_style_type = (ccc_control_service_style_type_e)style;
    }
    free(json_str);
  }
  return ret;
}

ccc_ctrl_msg_t ccc_dec_ctrl_msg_json(size_t len, uint8_t const ctrl_msg[len])
{
  assert(ctrl_msg != NULL);

  ccc_ctrl_msg_t ret = {0};

  if (len > 0) {
    char* json_str = ccc_sm_decode_ctrl_msg(ctrl_msg, len);
    if (json_str) {
      if (strstr(json_str, "\"listOfCellsControlled\"") != NULL) {
        ret.format = FORMAT_2_E2SM_CCC_CTRL_MSG;
        size_t slen = strlen(json_str);
        ret.format2.list_of_cells_controlled.data = json_str;
        ret.format2.list_of_cells_controlled.len  = slen;
      } else if (strstr(json_str, "\"listOfConfigurationStructures\"") != NULL) {
        ret.format = FORMAT_1_E2SM_CCC_CTRL_MSG;
        size_t slen = strlen(json_str);
        ret.format1.list_of_configuration_structures.data = json_str;
        ret.format1.list_of_configuration_structures.len  = slen;
      } else {
        ret.json_payload = json_str;
        ret.payload_len  = strlen(json_str);
      }
    }
  }
  return ret;
}

ccc_ctrl_out_t ccc_dec_ctrl_out_json(size_t len, uint8_t const ctrl_out[len])
{
  assert(ctrl_out != NULL);

  ccc_ctrl_out_t ret = {0};
  if (sscanf((const char*)ctrl_out, "{\"outcome\":%d}", &ret.outcome) != 1)
    ret.outcome = -1;

  return ret;
}

ccc_func_def_t ccc_dec_func_def_json(size_t len, uint8_t const func_def[len])
{
  assert(func_def != NULL);

  ccc_func_def_t ret = {0};
  if (len > 0) {
    ret.func_def = calloc(len, sizeof(uint8_t));
    assert(ret.func_def != NULL && "Memory exhausted");
    memcpy(ret.func_def, func_def, len);
    ret.len = len;
  }
  return ret;
}