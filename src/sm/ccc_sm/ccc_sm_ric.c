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

#include "ccc_sm_ric.h"
#include "ccc_sm_id.h"
#include "ie/ccc_data_ie.h"

#include "../sm_proc_data.h"
#include "../../util/alg_ds/alg/defer.h"
#include "../../util/byte_array.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  sm_ric_t base;

#ifdef CCC_SM_ENCODING_ASN
  ccc_enc_asn_t enc;
#elif CCC_SM_ENCODING_FLATBUFFERS
  ccc_enc_fb_t enc;
#else
  void* enc; // placeholder for plain encoding
#endif

} sm_ccc_ric_t;

static
sm_subs_data_t on_subscription_ccc_sm_ric(sm_ric_t const* sm_ric, void* cmd)
{
  assert(sm_ric != NULL); 
  assert(cmd != NULL); 
  sm_ccc_ric_t* sm = (sm_ccc_ric_t*)sm_ric;

  // For now, treat the cmd as a string - this may need to be adapted
  const char* cmd_str = (const char*)cmd;
  
  ccc_event_trigger_t et = {0};
  
  // Default to Format 3 (Periodic)
  et.format = FORMAT_3_E2SM_CCC_EV_TRIGGER_FORMAT;
  
  const int max_str_sz = 10;
  if(strlen(cmd_str) > max_str_sz)
    et.format3.properties.period.value = 1000;
  else
    et.format3.properties.period.value = atoi(cmd_str);

  printf("[CCC SM]: Event Trigger period = %ld ms\n", et.format3.properties.period.value);

  uint8_t* data = NULL; 
  size_t len_data = 0;

  // Encoding would go here for ASN/FB
  // For now, using basic implementation
  data = calloc(1, sizeof(ccc_event_trigger_t));
  assert(data != NULL);
  memcpy(data, &et, sizeof(ccc_event_trigger_t));
  len_data = sizeof(ccc_event_trigger_t);

  sm_subs_data_t dst = {0}; 
  
  // RIC Event Trigger Definition
  dst.event_trigger = data;  // Transfer ownership directly to pointer
  dst.len_et = len_data;

  // RIC Action Definition
  dst.action_def = NULL;
  dst.len_ad = 0;

  return dst;
}

static
sm_ag_if_rd_ind_t on_indication_ccc_sm_ric(sm_ric_t const* sm_ric, sm_ind_data_t const* data)
{
  assert(sm_ric != NULL); 
  assert(data != NULL); 

  sm_ccc_ric_t* sm = (sm_ccc_ric_t*)sm_ric;

  sm_ag_if_rd_ind_t dst = {0};
  dst.type = CCC_STATS_V0;

  // Initialize header (timestamp will be handled by framework)
  dst.ccc.hdr.timestamp = 0;  // Let the framework set this
  
  if(data->len_msg > 0 && data->ind_msg != NULL) {
    dst.ccc.msg.json_payload = calloc(data->len_msg + 1, sizeof(char));
    memcpy(dst.ccc.msg.json_payload, data->ind_msg, data->len_msg);
    dst.ccc.msg.payload_len = data->len_msg;
  }

  // Note: proc_id field doesn't exist in ccc_ind_data_t structure

  return dst;
}

static
sm_ctrl_req_data_t ric_on_control_req_ccc_sm_ric(sm_ric_t const* sm_ric, void* ctrl)
{
  assert(sm_ric != NULL); 
  assert(ctrl != NULL); 
  
  ccc_ctrl_req_data_t const* req = (ccc_ctrl_req_data_t const*)ctrl;
  sm_ccc_ric_t* sm = (sm_ccc_ric_t*)sm_ric;  

  sm_ctrl_req_data_t dst = {0};

  // Create dummy header and message for now
  dst.ctrl_hdr = calloc(4, sizeof(uint8_t));
  dst.len_hdr = 4;
  memcpy(dst.ctrl_hdr, &req->hdr.control_type, sizeof(req->hdr.control_type));

  // For control message, create simple payload
  size_t msg_len = sizeof(req->msg.payload_len);
  if(req->msg.json_payload != NULL) {
    msg_len += req->msg.payload_len;
  }
  
  dst.ctrl_msg = calloc(msg_len, sizeof(uint8_t));
  dst.len_msg = msg_len;
  
  // Copy payload length first, then payload if present
  memcpy(dst.ctrl_msg, &req->msg.payload_len, sizeof(req->msg.payload_len));
  if(req->msg.json_payload != NULL && req->msg.payload_len > 0) {
    memcpy(dst.ctrl_msg + sizeof(req->msg.payload_len), req->msg.json_payload, req->msg.payload_len);
  }

  return dst;
}

static
sm_ag_if_ans_ctrl_t ric_on_control_out_ccc_sm_ric(sm_ric_t const* sm_ric, sm_ctrl_out_data_t const* out)
{
  assert(sm_ric != NULL); 
  assert(out != NULL); 

  sm_ccc_ric_t* sm = (sm_ccc_ric_t*)sm_ric;

  sm_ag_if_ans_ctrl_t ag_if = {.type = CCC_AGENT_IF_CTRL_ANS_V0};  
  ag_if.ccc.outcome = 0; // Success

  return ag_if;
}

static
sm_ag_if_rd_e2setup_t ric_on_e2_setup_ccc_sm_ric(sm_ric_t const* sm_ric, sm_e2_setup_data_t const* setup)
{
  assert(sm_ric != NULL); 
  assert(setup != NULL); 

  sm_ag_if_rd_e2setup_t dst = {.type = CCC_AGENT_IF_E2_SETUP_ANS_V0}; 

  dst.ccc.func_def.len = setup->len_rfd;
  if(dst.ccc.func_def.len > 0){
    dst.ccc.func_def.func_def = calloc(dst.ccc.func_def.len, sizeof(uint8_t));
    assert(dst.ccc.func_def.func_def != NULL && "Memory exhausted");
    memcpy(dst.ccc.func_def.func_def, setup->ran_fun_def, setup->len_rfd);
  }

  return dst;
}

static
sm_ag_if_rd_rsu_t on_ric_service_update_ccc_sm_ric(sm_ric_t const* sm_ric, sm_ric_service_update_data_t const* data)
{
  assert(sm_ric != NULL); 
  assert(data != NULL); 

  assert(0!=0 && "Not implemented");
}

static
void free_ccc_sm_ric(sm_ric_t* sm_ric)
{
  assert(sm_ric != NULL);
  sm_ccc_ric_t* sm = (sm_ccc_ric_t*)sm_ric;
  free(sm);
}

//
// Allocation SM functions. The memory malloc by the SM is also freed by it.
//

static
void free_subs_data_ccc_sm_ric(void* msg)
{
  assert(msg != NULL);
  assert(0!=0 && "Not implemented");
}

static
void free_ind_data_ccc_sm_ric(void* msg)
{
  assert(msg != NULL);
  assert(0!=0 && "Not implemented");
}

static
void free_ctrl_req_data_ccc_sm_ric(void* msg)
{
  assert(msg != NULL);
  assert(0!=0 && "Not implemented");
}

static
void free_ctrl_out_data_ccc_sm_ric(void* msg)
{
  assert(msg != NULL);
  assert(0!=0 && "Not implemented");
}

static
void free_e2_setup_ccc_sm_ric(void* msg)
{
  assert(msg != NULL);
  assert(0!=0 && "Not implemented");
}

void free_ric_service_update_ccc_sm_ric(void* msg)
{
  assert(msg != NULL);
  assert(0!=0 && "Not implemented");
}

sm_ric_t* make_ccc_sm_ric(void)
{
  sm_ccc_ric_t* sm = calloc(1, sizeof(sm_ccc_ric_t));
  assert(sm != NULL && "Memory exhausted");

  *((uint16_t*)&sm->base.ran_func_id) = SM_CCC_ID; 

  sm->base.free_sm = free_ccc_sm_ric;

  // Memory (De)Allocation
  sm->base.alloc.free_subs_data_msg = free_subs_data_ccc_sm_ric; 
  sm->base.alloc.free_ind_data = free_ind_data_ccc_sm_ric; 
  sm->base.alloc.free_ctrl_req_data = free_ctrl_req_data_ccc_sm_ric; 
  sm->base.alloc.free_ctrl_out_data = free_ctrl_out_data_ccc_sm_ric; 
 
  sm->base.alloc.free_e2_setup = free_e2_setup_ccc_sm_ric; 
  sm->base.alloc.free_ric_service_update = free_ric_service_update_ccc_sm_ric; 

  // O-RAN E2SM 5 Procedures
  sm->base.proc.on_subscription = on_subscription_ccc_sm_ric; 
  sm->base.proc.on_indication = on_indication_ccc_sm_ric;

  sm->base.proc.on_control_req = ric_on_control_req_ccc_sm_ric;
  sm->base.proc.on_control_out = ric_on_control_out_ccc_sm_ric;

  sm->base.proc.on_e2_setup = ric_on_e2_setup_ccc_sm_ric;
  sm->base.proc.on_ric_service_update = on_ric_service_update_ccc_sm_ric; 
  sm->base.handle = NULL;

  assert(strlen(SM_CCC_STR) < sizeof(sm->base.ran_func_name));
  memcpy(sm->base.ran_func_name, SM_CCC_STR, strlen(SM_CCC_STR)); 

  return &sm->base;
}

uint16_t id_sm_ccc_ric(sm_ric_t const* sm_ric)
{
  assert(sm_ric != NULL);
  sm_ccc_ric_t* sm = (sm_ccc_ric_t*)sm_ric;
  return sm->base.ran_func_id;
}
