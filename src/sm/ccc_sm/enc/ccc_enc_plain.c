
#include "ccc_enc_plain.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

byte_array_t ccc_enc_event_trigger_plain(ccc_event_trigger_t const* event_trigger)
{
  assert(event_trigger != NULL);
  
  byte_array_t ba = {0};
  
  ba.len = sizeof(ccc_event_trigger_t);
  ba.buf = malloc(ba.len);
  assert(ba.buf != NULL && "Memory exhausted");
  
  memcpy(ba.buf, event_trigger, ba.len);
  
  return ba;
}

byte_array_t ccc_enc_action_def_plain(ccc_action_def_t const* action_def)
{
  assert(action_def != NULL);
  
  byte_array_t ba = {0};
  
  ba.len = sizeof(ccc_action_def_t);
  ba.buf = malloc(ba.len);
  assert(ba.buf != NULL && "Memory exhausted");
  
  memcpy(ba.buf, action_def, ba.len);
  
  return ba;
}

byte_array_t ccc_enc_ind_hdr_plain(ccc_ind_hdr_t const* ind_hdr)
{
  assert(ind_hdr != NULL);
  
  char json_buffer[512];
  int len = snprintf(json_buffer, sizeof(json_buffer),
      "{\"timestamp\":%u}",
      ind_hdr->timestamp);
  
  assert(len > 0 && len < (int)sizeof(json_buffer) && "JSON buffer overflow");
  
  byte_array_t ba = {0};
  ba.len = len + 1;
  ba.buf = malloc(ba.len);
  assert(ba.buf != NULL && "Memory exhausted");
  
  memcpy(ba.buf, json_buffer, ba.len);
  
  return ba;
}

byte_array_t ccc_enc_ind_msg_plain(ccc_ind_msg_t const* ind_msg)
{
  assert(ind_msg != NULL);
  
  byte_array_t ba = {0};
  
  if(ind_msg->json_payload != NULL && ind_msg->payload_len > 0) {
    ba.len = ind_msg->payload_len;
    ba.buf = malloc(ba.len);
    assert(ba.buf != NULL && "Memory exhausted");
    
    memcpy(ba.buf, ind_msg->json_payload, ba.len);
  }
  
  return ba;
}

byte_array_t ccc_enc_ctrl_hdr_plain(ccc_ctrl_hdr_t const* ctrl_hdr)
{
  assert(ctrl_hdr != NULL);
  
  char json_buffer[512];
  int len = 0;
  
  // Encode based on format
  if (ctrl_hdr->format == FORMAT_1_E2SM_CCC_CTRL_HDR) {
    // Format 1: Include format and ric_style_type
    len = snprintf(json_buffer, sizeof(json_buffer),
        "{"
        "\"format\":%d,"
        "\"format1\":{"
        "\"ric_style_type\":%u"
        "}"
        "}",
        ctrl_hdr->format,
        ctrl_hdr->format1.ric_style_type);
  } else {
    // Backward compatible: simple control_type
    len = snprintf(json_buffer, sizeof(json_buffer),
        "{\"control_type\":%u}",
        ctrl_hdr->control_type);
  }
  
  assert(len > 0 && len < (int)sizeof(json_buffer) && "JSON buffer overflow");
  
  byte_array_t ba = {0};
  ba.len = len;
  ba.buf = malloc(ba.len);
  assert(ba.buf != NULL && "Memory exhausted");
  
  memcpy(ba.buf, json_buffer, ba.len);
  
  return ba;
}

byte_array_t ccc_enc_ctrl_msg_plain(ccc_ctrl_msg_t const* ctrl_msg)
{
  assert(ctrl_msg != NULL);
  
  byte_array_t ba = {0};
  
  // Encode based on format
  if (ctrl_msg->format == FORMAT_1_E2SM_CCC_CTRL_MSG) {
    // Format 1: Node-level configuration structures
    if (ctrl_msg->format1.list_of_configuration_structures.data != NULL &&
        ctrl_msg->format1.list_of_configuration_structures.len > 0) {
      ba.len = ctrl_msg->format1.list_of_configuration_structures.len;
      ba.buf = malloc(ba.len);
      assert(ba.buf != NULL && "Memory exhausted");
      
      memcpy(ba.buf, ctrl_msg->format1.list_of_configuration_structures.data, ba.len);
    }
  } 
  else if (ctrl_msg->format == FORMAT_2_E2SM_CCC_CTRL_MSG) {
    // Format 2: Cell-level configuration
    if (ctrl_msg->format2.list_of_cells_controlled.data != NULL &&
        ctrl_msg->format2.list_of_cells_controlled.len > 0) {
      ba.len = ctrl_msg->format2.list_of_cells_controlled.len;
      ba.buf = malloc(ba.len);
      assert(ba.buf != NULL && "Memory exhausted");
      
      memcpy(ba.buf, ctrl_msg->format2.list_of_cells_controlled.data, ba.len);
    }
  }
  else {
    // Backward compatible: simple json_payload
    if (ctrl_msg->json_payload != NULL && ctrl_msg->payload_len > 0) {
      ba.len = ctrl_msg->payload_len;
      ba.buf = malloc(ba.len);
      assert(ba.buf != NULL && "Memory exhausted");
      
      memcpy(ba.buf, ctrl_msg->json_payload, ba.len);
    }
  }
  
  return ba;
}

byte_array_t ccc_enc_ctrl_out_plain(ccc_ctrl_out_t const* ctrl_out)
{
  assert(ctrl_out != NULL);
  
  char json_buffer[1024];
  int len = snprintf(json_buffer, sizeof(json_buffer),
      "{\"outcome\":%d}",
      ctrl_out->outcome);
  
  assert(len > 0 && len < (int)sizeof(json_buffer) && "JSON buffer overflow");
  
  byte_array_t ba = {0};
  ba.len = len + 1;
  ba.buf = malloc(ba.len);
  assert(ba.buf != NULL && "Memory exhausted");
  
  memcpy(ba.buf, json_buffer, ba.len);
  
  return ba;
}

byte_array_t ccc_enc_func_def_plain(ccc_func_def_t const* func_def)
{
  assert(func_def != NULL);
  
  byte_array_t ba = {0};
  
  if(func_def->func_def != NULL && func_def->len > 0) {
    ba.len = func_def->len;
    ba.buf = malloc(ba.len);
    assert(ba.buf != NULL && "Memory exhausted");
    
    memcpy(ba.buf, func_def->func_def, ba.len);
  }
  
  return ba;
}

