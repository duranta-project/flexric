/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include "ccc_enc_json.h"
#include "../ccc_sm.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

byte_array_t ccc_enc_event_trigger_json(ccc_event_trigger_t const* event_trigger)
{
  assert(event_trigger != NULL);

  byte_array_t ba = {0};
  ba.len = sizeof(ccc_event_trigger_t);
  ba.buf = malloc(ba.len);
  assert(ba.buf != NULL && "Memory exhausted");
  memcpy(ba.buf, event_trigger, ba.len);
  return ba;
}

byte_array_t ccc_enc_action_def_json(ccc_action_def_t const* action_def)
{
  assert(action_def != NULL);

  byte_array_t ba = {0};
  ba.len = sizeof(ccc_action_def_t);
  ba.buf = malloc(ba.len);
  assert(ba.buf != NULL && "Memory exhausted");
  memcpy(ba.buf, action_def, ba.len);
  return ba;
}

byte_array_t ccc_enc_ind_hdr_json(ccc_ind_hdr_t const* ind_hdr)
{
  assert(ind_hdr != NULL);

  int64_t     evt_time = 0;
  const char* reason   = NULL;

  if (ind_hdr->format == FORMAT_1_E2SM_CCC_IND_HDR) {
    evt_time = (int64_t)ind_hdr->format1.event_time;
    reason   = ind_hdr->format1.indication_reason;
  } else {
    evt_time = (int64_t)ind_hdr->timestamp;
  }

  size_t   out_len = 0;
  uint8_t* buf     = ccc_sm_encode_ind_hdr(evt_time, reason ? reason : "", &out_len);

  byte_array_t ba = {0};
  if (buf && out_len > 0) { ba.buf = buf; ba.len = out_len; }
  return ba;
}

byte_array_t ccc_enc_ind_msg_json(ccc_ind_msg_t const* ind_msg)
{
  assert(ind_msg != NULL);

  const char* payload     = NULL;
  size_t      payload_len = 0;

  if (ind_msg->format == FORMAT_1_E2SM_CCC_IND_MSG) {
    payload     = ind_msg->format1.list_of_configuration_structures_reported.data;
    payload_len = ind_msg->format1.list_of_configuration_structures_reported.len;
  } else if (ind_msg->format == FORMAT_2_E2SM_CCC_IND_MSG) {
    payload     = ind_msg->format2.list_of_cells_reported.data;
    payload_len = ind_msg->format2.list_of_cells_reported.len;
  } else if (ind_msg->json_payload != NULL && ind_msg->payload_len > 0) {
    payload     = ind_msg->json_payload;
    payload_len = ind_msg->payload_len;
  }

  byte_array_t ba = {0};
  if (payload && payload_len > 0) {
    size_t   out_len = 0;
    uint8_t* buf     = ccc_sm_encode_ind_msg(payload, payload_len, &out_len);
    if (buf && out_len > 0) { ba.buf = buf; ba.len = out_len; }
  }
  return ba;
}

byte_array_t ccc_enc_ctrl_hdr_json(ccc_ctrl_hdr_t const* ctrl_hdr)
{
  assert(ctrl_hdr != NULL);

  int style_type = 0;
  if (ctrl_hdr->format == FORMAT_1_E2SM_CCC_CTRL_HDR) {
    style_type = (int)ctrl_hdr->format1.ric_style_type;
  } else {
    style_type = (int)ctrl_hdr->control_type;
  }

  size_t   out_len = 0;
  uint8_t* buf     = ccc_sm_encode_ctrl_hdr(style_type, &out_len);

  byte_array_t ba = {0};
  if (buf && out_len > 0) { ba.buf = buf; ba.len = out_len; }
  return ba;
}

byte_array_t ccc_enc_ctrl_msg_json(ccc_ctrl_msg_t const* ctrl_msg)
{
  assert(ctrl_msg != NULL);

  const char* payload     = NULL;
  size_t      payload_len = 0;

  if (ctrl_msg->format == FORMAT_1_E2SM_CCC_CTRL_MSG) {
    payload     = ctrl_msg->format1.list_of_configuration_structures.data;
    payload_len = ctrl_msg->format1.list_of_configuration_structures.len;
  } else if (ctrl_msg->format == FORMAT_2_E2SM_CCC_CTRL_MSG) {
    payload     = ctrl_msg->format2.list_of_cells_controlled.data;
    payload_len = ctrl_msg->format2.list_of_cells_controlled.len;
  } else if (ctrl_msg->json_payload != NULL && ctrl_msg->payload_len > 0) {
    payload     = ctrl_msg->json_payload;
    payload_len = ctrl_msg->payload_len;
  }

  byte_array_t ba = {0};
  if (payload && payload_len > 0) {
    size_t   out_len = 0;
    uint8_t* buf     = ccc_sm_encode_ctrl_msg(payload, payload_len, &out_len);
    if (buf && out_len > 0) { ba.buf = buf; ba.len = out_len; }
  }
  return ba;
}

byte_array_t ccc_enc_ctrl_out_json(ccc_ctrl_out_t const* ctrl_out)
{
  assert(ctrl_out != NULL);

  /*
   * Format 1 outcome keys per E2SmCccControlOutcomeFormat1Properties:
   *   "receivedTimestamp", "ranConfigurationStructuresAcceptedList",
   *   "ranConfigurationStructuresFailedList"
   */
  const char* payload     = NULL;
  size_t      payload_len = 0;

  if (ctrl_out->format == FORMAT_1_E2SM_CCC_CTRL_OUT) {
    payload     = ctrl_out->format1.ran_configuration_structures_accepted_list.data;
    payload_len = ctrl_out->format1.ran_configuration_structures_accepted_list.len;
  } else if (ctrl_out->format == FORMAT_2_E2SM_CCC_CTRL_OUT) {
    payload     = ctrl_out->format2.list_of_cells_for_control_outcome.data;
    payload_len = ctrl_out->format2.list_of_cells_for_control_outcome.len;
  }

  byte_array_t ba = {0};
  if (payload && payload_len > 0) {
    ba.len = payload_len;
    ba.buf = malloc(ba.len);
    assert(ba.buf != NULL && "Memory exhausted");
    memcpy(ba.buf, payload, ba.len);
  } else {
    char json_buf[64];
    int  len = snprintf(json_buf, sizeof(json_buf),
                        "{\"outcome\":%d}", ctrl_out->outcome);
    ba.len = (size_t)len;
    ba.buf = malloc(ba.len + 1);
    assert(ba.buf != NULL && "Memory exhausted");
    memcpy(ba.buf, json_buf, ba.len + 1);
  }
  return ba;
}

byte_array_t ccc_enc_func_def_json(ccc_func_def_t const* func_def)
{
  assert(func_def != NULL);

  byte_array_t ba = {0};
  if (func_def->func_def != NULL && func_def->len > 0) {
    ba.len = func_def->len;
    ba.buf = malloc(ba.len);
    assert(ba.buf != NULL && "Memory exhausted");
    memcpy(ba.buf, func_def->func_def, ba.len);
  }
  return ba;
}
