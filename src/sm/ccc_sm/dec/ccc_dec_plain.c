/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.openairinterface.org/?page_id=698
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */

#include "ccc_dec_plain.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

ccc_event_trigger_t ccc_dec_event_trigger_plain(size_t len, uint8_t const ev_tr[len])
{
  assert(len >= sizeof(ccc_event_trigger_t));
  assert(ev_tr != NULL);
  
  ccc_event_trigger_t ret = {0};
  memcpy(&ret, ev_tr, sizeof(ccc_event_trigger_t));
  
  return ret;
}

ccc_action_def_t ccc_dec_action_def_plain(size_t len, uint8_t const action_def[len])
{
  assert(len >= sizeof(ccc_action_def_t));
  assert(action_def != NULL);
  
  ccc_action_def_t ret = {0};
  memcpy(&ret, action_def, sizeof(ccc_action_def_t));
  
  return ret;
}

ccc_ind_hdr_t ccc_dec_ind_hdr_plain(size_t len, uint8_t const ind_hdr[len])
{
  assert(ind_hdr != NULL);
  
  ccc_ind_hdr_t ret = {0};
  
  const char* json_str = (const char*)ind_hdr;
  
  if(sscanf(json_str, "{\"timestamp\":%u}", &ret.timestamp) != 1) {
    ret.timestamp = 0;
  }
  
  return ret;
}

ccc_ind_msg_t ccc_dec_ind_msg_plain(size_t len, uint8_t const ind_msg[len])
{
  assert(ind_msg != NULL);
  
  ccc_ind_msg_t ret = {0};
  
  if(len > 0) {
    ret.json_payload = calloc(len + 1, sizeof(char));
    assert(ret.json_payload != NULL && "Memory exhausted");
    
    memcpy(ret.json_payload, ind_msg, len);
    ret.json_payload[len] = '\0';
    ret.payload_len = len;
  }
  
  return ret;
}

ccc_ctrl_hdr_t ccc_dec_ctrl_hdr_plain(size_t len, uint8_t const ctrl_hdr[len])
{
  assert(ctrl_hdr != NULL);
  
  ccc_ctrl_hdr_t ret = {0};
  
  const char* json_str = (const char*)ctrl_hdr;
  
  // Try to parse Format 1 JSON: {"format":1,"format1":{"ric_style_type":X}}
  int format = 0;
  unsigned int ric_style_type = 0;
  
  if (sscanf(json_str, "{\"format\":%d,\"format1\":{\"ric_style_type\":%u}}", 
             &format, &ric_style_type) == 2) {
    // Successfully parsed Format 1
    ret.format = (e2sm_ccc_ctrl_hdr_format_e)format;
    ret.format1.ric_style_type = ric_style_type;
  }
  else if (sscanf(json_str, "{\"control_type\":%u}", &ret.control_type) == 1) {
    // Backward compatible: simple control_type
    ret.format = FORMAT_1_E2SM_CCC_CTRL_HDR;  // Default
  }
  else {
    // Failed to parse
    ret.control_type = 0;
  }
  
  return ret;
}

ccc_ctrl_msg_t ccc_dec_ctrl_msg_plain(size_t len, uint8_t const ctrl_msg[len])
{
  assert(ctrl_msg != NULL);
  
  ccc_ctrl_msg_t ret = {0};
  
  if (len > 0) {
    // Try to detect format from JSON content
    const char* json_str = (const char*)ctrl_msg;
    
    // Check if it contains "list_of_cells_controlled" (Format 2)
    if (strstr(json_str, "list_of_cells_controlled") != NULL) {
      ret.format = FORMAT_2_E2SM_CCC_CTRL_MSG;
      ret.format2.list_of_cells_controlled.data = calloc(len + 1, sizeof(char));
      assert(ret.format2.list_of_cells_controlled.data != NULL && "Memory exhausted");
      
      memcpy(ret.format2.list_of_cells_controlled.data, ctrl_msg, len);
      ret.format2.list_of_cells_controlled.data[len] = '\0';
      ret.format2.list_of_cells_controlled.len = len;
    }
    // Check if it contains "list_of_configuration_structures" at root (Format 1)
    else if (strstr(json_str, "\"list_of_configuration_structures\"") != NULL &&
             strstr(json_str, "list_of_cells_controlled") == NULL) {
      ret.format = FORMAT_1_E2SM_CCC_CTRL_MSG;
      ret.format1.list_of_configuration_structures.data = calloc(len + 1, sizeof(char));
      assert(ret.format1.list_of_configuration_structures.data != NULL && "Memory exhausted");
      
      memcpy(ret.format1.list_of_configuration_structures.data, ctrl_msg, len);
      ret.format1.list_of_configuration_structures.data[len] = '\0';
      ret.format1.list_of_configuration_structures.len = len;
    }
    else {
      // Backward compatible: store as json_payload
      ret.json_payload = calloc(len + 1, sizeof(char));
      assert(ret.json_payload != NULL && "Memory exhausted");
      
      memcpy(ret.json_payload, ctrl_msg, len);
      ret.json_payload[len] = '\0';
      ret.payload_len = len;
    }
  }
  
  return ret;
}

ccc_ctrl_out_t ccc_dec_ctrl_out_plain(size_t len, uint8_t const ctrl_out[len])
{
  assert(ctrl_out != NULL);
  
  ccc_ctrl_out_t ret = {0};
  
  const char* json_str = (const char*)ctrl_out;
  
  if(sscanf(json_str, "{\"outcome\":%d}", &ret.outcome) != 1) {
    ret.outcome = -1;
  }
  
  return ret;
}

ccc_func_def_t ccc_dec_func_def_plain(size_t len, uint8_t const func_def[len])
{
  assert(func_def != NULL);
  
  ccc_func_def_t ret = {0};
  
  if(len > 0) {
    ret.func_def = calloc(len, sizeof(uint8_t));
    assert(ret.func_def != NULL && "Memory exhausted");
    
    memcpy(ret.func_def, func_def, len);
    ret.len = len;
  }
  
  return ret;
}

