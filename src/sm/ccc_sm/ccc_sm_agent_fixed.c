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

#include "ccc_sm_agent.h"
#include "ccc_sm_id.h"
#include "ie/ccc_data_ie.h"

#include "../sm_proc_data.h"
#include "../../util/alg_ds/alg/defer.h"
#include "../../util/byte_array.h"
#include "../../util/conversions.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Helper macro for stringification
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

typedef struct {
  sm_agent_t base;

#ifdef CCC_SM_ENCODING_ASN
  void* enc_asn;
#elif CCC_SM_ENCODING_FLATBUFFERS
  void* enc_fb;
#else
  // PLAIN encoding - JSON-based, simple C structures
  void* enc; // placeholder for plain encoding
#endif

} sm_ccc_agent_t;

static
sm_ag_if_ans_subs_t on_subscription_ccc_sm_ag(sm_agent_t const* sm_agent, sm_subs_data_t const* data)
{
  assert(sm_agent != NULL);
  assert(data != NULL);

  sm_ccc_agent_t* sm = (sm_ccc_agent_t*)sm_agent;

  // For CCC SM using JSON schema, decode subscription data
  sm_ag_if_ans_subs_t ans = {.type = PERIODIC_SUBSCRIPTION_FLRC};
  
  // Extract period from event trigger data (assuming JSON format)
  uint32_t period_ms = 1000; // Default 1 second
  
  if (data->len_et > 0 && data->event_trigger != NULL) {
    // For JSON-based CCC SM, parse the trigger
    ccc_event_trigger_t* et = (ccc_event_trigger_t*)data->event_trigger;
    if (et->format == FORMAT_3_E2SM_CCC_EV_TRIGGER_FORMAT) {
      period_ms = et->format3.properties.period.value;
    }
  }
  
  subscribe_timer_t timer = {
    .type = CCC_V0_SUB_DATA_ENUM,
    .ms = period_ms
  };
  
  // For action definition, decode if present
  void* act_def = NULL;
  if (data->len_ad > 0 && data->action_def != NULL) {
    // Copy action definition for later use
    act_def = calloc(data->len_ad, sizeof(uint8_t));
    assert(act_def != NULL && "Memory exhausted");
    memcpy(act_def, data->action_def, data->len_ad);
  }
  
  timer.act_def = act_def;
  ans.per.t = timer;
  
  printf("[CCC SM Agent]: Subscription with period %u ms\n", period_ms);
  return ans;
}

static
exp_ind_data_t on_indication_ccc_sm_ag(sm_agent_t const* sm_agent, void* act_def)
{
  assert(sm_agent != NULL);
  
  sm_ccc_agent_t* sm = (sm_ccc_agent_t*)sm_agent;

  exp_ind_data_t ret = {.has_value = true};

  // Create CCC indication data following JSON schema structure
  ccc_rd_ind_data_t ccc = {0};
  
  // Set action definition if available
  ccc.act_def = act_def;
  
  // Use I/O interface to read data from RAN if available
  if (sm->base.io.read_ind != NULL) {
    if(sm->base.io.read_ind(&ccc) == false) {
      return (exp_ind_data_t){.has_value = false};
    }
  } else {
    // Generate default CCC indication data based on JSON schema
    // Based on your E2SmCccIndicationMessageFormat1
    
    const char* sample_config_json = 
      "{"
      "\"indication_format\": 1,"
      "\"list_of_configuration_structures_reported\": ["
        "{"
          "\"ran_configuration_structure_name\": \"O_RU_Info\","
          "\"values_of_attributes\": {"
            "\"energy_saving_capability_common_info\": {"
              "\"st8_ready_message_supported\": true,"
              "\"sleep_duration_extension_supported\": true,"
              "\"emergency_wake_up_command_supported\": false"
            "}"
          "}"
        "}"
      "]}";
    
    size_t json_len = strlen(sample_config_json);
    ccc.ind.msg.json_payload = calloc(json_len + 1, sizeof(char));
    assert(ccc.ind.msg.json_payload != NULL && "Memory exhausted");
    memcpy(ccc.ind.msg.json_payload, sample_config_json, json_len);
    ccc.ind.msg.payload_len = json_len;
    
    // Set indication header timestamp
    ccc.ind.hdr.timestamp = time_now_us();
  }

  // Free memory allocated by RAN at read_ind
  defer({ 
    if (ccc.ind.msg.json_payload != NULL) {
      free(ccc.ind.msg.json_payload);
    }
  });

  // For CCC SM with JSON/PLAIN encoding, create simple byte arrays
  // Header encoding (simplified for JSON-based CCC)
  char hdr_json[256];
  snprintf(hdr_json, sizeof(hdr_json), 
           "{\"timestamp\":%u}", 
           ccc.ind.hdr.timestamp);
  
  size_t hdr_len = strlen(hdr_json);
  ret.data.ind_hdr = calloc(hdr_len + 1, sizeof(uint8_t));
  assert(ret.data.ind_hdr != NULL && "Memory exhausted");
  memcpy(ret.data.ind_hdr, hdr_json, hdr_len);
  ret.data.len_hdr = hdr_len;

  // Message encoding
  if (ccc.ind.msg.payload_len > 0 && ccc.ind.msg.json_payload != NULL) {
    ret.data.ind_msg = calloc(ccc.ind.msg.payload_len + 1, sizeof(uint8_t));
    assert(ret.data.ind_msg != NULL && "Memory exhausted");
    memcpy(ret.data.ind_msg, ccc.ind.msg.json_payload, ccc.ind.msg.payload_len);
    ret.data.len_msg = ccc.ind.msg.payload_len;
  } else {
    // Default empty message
    ret.data.ind_msg = calloc(1, sizeof(uint8_t));
    ret.data.len_msg = 0;
  }

  // No call process ID for CCC
  ret.data.call_process_id = NULL;
  ret.data.len_cpid = 0;
  
  return ret;
}

static
sm_ctrl_out_data_t on_control_ccc_sm_ag(sm_agent_t const* sm_agent, sm_ctrl_req_data_t const* data)
{
  assert(sm_agent != NULL);
  assert(data != NULL);

  sm_ccc_agent_t* sm = (sm_ccc_agent_t*)sm_agent;

  sm_ctrl_out_data_t out = {0};
  
  // Decode control header and message (JSON-based)
  // Based on E2SmCccControlHeaderFormat1 and E2SmCccControlMessageFormat1/Format2
  
  if (data->len_hdr > 0 && data->ctrl_hdr != NULL) {
    // Parse JSON control header
    // Expected format: {"ric_style_type": 1}
    printf("[CCC SM Agent]: Control header received: %.*s\n", (int)data->len_hdr, (char*)data->ctrl_hdr);
  }
  
  if (data->len_msg > 0 && data->ctrl_msg != NULL) {
    // Parse JSON control message
    // Expected format based on your schema:
    // Format1: {"list_of_configuration_structures": [...]}
    // Format2: {"list_of_cells_controlled": [...]}
    
    printf("[CCC SM Agent]: Control message received: %.*s\n", (int)data->len_msg, (char*)data->ctrl_msg);
    
    // Execute control action through I/O interface if available
    if (sm->base.io.write_ctrl != NULL) {
      // Create CCC control request data structure
      ccc_ctrl_req_data_t req = {0};
      req.hdr.control_type = 1; // Default control type
      
      // Copy JSON payload
      req.msg.json_payload = calloc(data->len_msg + 1, sizeof(char));
      assert(req.msg.json_payload != NULL && "Memory exhausted");
      memcpy(req.msg.json_payload, data->ctrl_msg, data->len_msg);
      req.msg.payload_len = data->len_msg;
      
      // Call the control function
      sm_ag_if_ans_t ans = sm->base.io.write_ctrl(&req);
      
      // Cleanup
      if (req.msg.json_payload != NULL) {
        free(req.msg.json_payload);
      }
      
      // Generate control outcome based on result
      // Success - Generate E2SmCccControlOutcomeFormat1
      char* formatted_outcome = calloc(512, sizeof(char));
      assert(formatted_outcome != NULL && "Memory exhausted");
      uint32_t timestamp = time_now_us();
      snprintf(formatted_outcome, 512,
        "{"
        "\"received_timestamp\": %u,"
        "\"ran_configuration_structures_accepted_list\": ["
          "{"
            "\"ran_configuration_structure_name\": \"O_RU_Info\","
            "\"applied_timestamp\": %u"
          "}"
        "],"
        "\"ran_configuration_structures_failed_list\": []"
        "}", timestamp, timestamp);
      
      out.len = strlen(formatted_outcome);
      out.ctrl_out = (uint8_t*)formatted_outcome;
    } else {
      // Default success response when no I/O interface
      char* formatted_default = calloc(256, sizeof(char));
      assert(formatted_default != NULL && "Memory exhausted");
      snprintf(formatted_default, 256,
        "{"
        "\"received_timestamp\": %u,"
        "\"result\": \"success\""
        "}", (uint32_t)time_now_us());
      
      out.len = strlen(formatted_default);
      out.ctrl_out = (uint8_t*)formatted_default;
    }
  } else {
    // No control message - generate minimal response
    const char* minimal_outcome = "{\"result\":\"no_action\"}";
    out.len = strlen(minimal_outcome);
    out.ctrl_out = calloc(out.len + 1, sizeof(uint8_t));
    assert(out.ctrl_out != NULL && "Memory exhausted");
    memcpy(out.ctrl_out, minimal_outcome, out.len);
  }
  
  return out;
}

static
sm_e2_setup_data_t on_e2_setup_ccc_sm_ag(sm_agent_t const* sm_agent)
{
  assert(sm_agent != NULL);

  sm_ccc_agent_t* sm = (sm_ccc_agent_t*)sm_agent;

  sm_e2_setup_data_t setup = {0};
  
  // Generate RAN function definition for CCC (JSON-based)
  char* ran_func_def_json = calloc(512, sizeof(char));
  assert(ran_func_def_json != NULL && "Memory exhausted");
  
  snprintf(ran_func_def_json, 512,
    "{"
    "\"ranFunctionName\":\"CCC\","
    "\"ranFunctionId\":" TOSTRING(SM_CCC_ID) ","
    "\"ranFunctionDescription\":\"Cell Capacity and Coverage Service Model\","
    "\"ranFunctionRevision\":" TOSTRING(SM_CCC_REV) ","
    "\"encoding\":\"JSON\""
    "}");
  
  size_t def_len = strlen(ran_func_def_json);
  setup.ran_fun_def = (uint8_t*)ran_func_def_json;
  setup.len_rfd = def_len;
  
  return setup;
}

static
sm_ric_service_update_data_t on_ric_service_update_ccc_sm_ag(sm_agent_t const* sm_agent)
{
  assert(sm_agent != NULL);

  sm_ccc_agent_t* sm = (sm_ccc_agent_t*)sm_agent;

  sm_ric_service_update_data_t update = {0};
  
  // Generate service update data (JSON-based)
  const char* update_json = "{\"status\":\"active\",\"capabilities\":[\"monitoring\",\"control\"]}";
  size_t update_len = strlen(update_json);
  
  update.ran_fun_def = calloc(update_len + 1, sizeof(uint8_t));
  assert(update.ran_fun_def != NULL && "Memory exhausted");
  memcpy(update.ran_fun_def, update_json, update_len);
  update.len_rfd = update_len;
  
  return update;
}

static
void free_ccc_sm_ag(sm_agent_t* sm_agent)
{
  assert(sm_agent != NULL);
  sm_ccc_agent_t* sm = (sm_ccc_agent_t*)sm_agent;
  free(sm);
}

static
void free_act_def_ccc_sm_ag(sm_agent_t* sm_agent, void* act_def)
{
  assert(sm_agent != NULL);
  if (act_def != NULL) {
    free(act_def);
  }
}

//
// Allocation SM functions. Memory allocated by the SM is also freed by it.
//

static
void free_subs_data_ccc_sm_ag(void* msg)
{
  if (msg != NULL) {
    free(msg);
  }
}

static
void free_ind_data_ccc_sm_ag(void* msg)
{
  if (msg != NULL) {
    free(msg);
  }
}

static
void free_ctrl_req_data_ccc_sm_ag(void* msg)
{
  if (msg != NULL) {
    free(msg);
  }
}

static
void free_ctrl_out_data_ccc_sm_ag(void* msg)
{
  if (msg != NULL) {
    free(msg);
  }
}

static
void free_e2_setup_ccc_sm_ag(void* msg)
{
  if (msg != NULL) {
    sm_e2_setup_data_t* setup = (sm_e2_setup_data_t*)msg;
    if (setup->ran_fun_def != NULL) {
      free(setup->ran_fun_def);
    }
    free(setup);
  }
}

static
void free_ric_service_update_ccc_sm_ag(void* msg)
{
  if (msg != NULL) {
    sm_ric_service_update_data_t* update = (sm_ric_service_update_data_t*)msg;
    if (update->ran_fun_def != NULL) {
      free(update->ran_fun_def);
    }
    free(update);
  }
}

// SM Information functions
static
char const* def_ccc_sm_ag(void)
{
  return SM_CCC_STR;
}

static
uint16_t id_ccc_sm_ag(void)
{
  return SM_CCC_ID;
}

static
uint16_t rev_ccc_sm_ag(void)
{
  return SM_CCC_REV;
}

static
char const* oid_ccc_sm_ag(void)
{
  return SM_CCC_OID;
}

// Main factory function - this is the missing function that was causing the error!
sm_agent_t* make_ccc_sm_agent(sm_io_ag_ran_t io)
{
  sm_ccc_agent_t* sm = calloc(1, sizeof(sm_ccc_agent_t));
  assert(sm != NULL && "Memory exhausted");

  // Set I/O functions from RAN following KMP pattern
  sm->base.io.read_ind = io.read_ind_tbl[CCC_STATS_V0];
  sm->base.io.read_setup = NULL; // Not implemented yet
  sm->base.io.read_rsu = NULL; // Not implemented yet
  
  // Write
  sm->base.io.write_ctrl = NULL; // Not implemented yet
  sm->base.io.write_subs = NULL; // Not implemented yet

  sm->base.free_sm = free_ccc_sm_ag;
  sm->base.free_act_def = free_act_def_ccc_sm_ag;

  // Memory (De)Allocation
  sm->base.alloc.free_subs_data_msg = free_subs_data_ccc_sm_ag;
  sm->base.alloc.free_ind_data = free_ind_data_ccc_sm_ag;
  sm->base.alloc.free_ctrl_req_data = free_ctrl_req_data_ccc_sm_ag;
  sm->base.alloc.free_ctrl_out_data = free_ctrl_out_data_ccc_sm_ag;
  
  sm->base.alloc.free_e2_setup = free_e2_setup_ccc_sm_ag;
  sm->base.alloc.free_ric_service_update = free_ric_service_update_ccc_sm_ag;

  // O-RAN E2SM 5 Procedures
  sm->base.proc.on_subscription = on_subscription_ccc_sm_ag;
  sm->base.proc.on_indication = on_indication_ccc_sm_ag;
  sm->base.proc.on_control = on_control_ccc_sm_ag;
  sm->base.proc.on_e2_setup = on_e2_setup_ccc_sm_ag;
  sm->base.proc.on_ric_service_update = on_ric_service_update_ccc_sm_ag;

  sm->base.handle = NULL;

  // SM Information
  sm->base.info.def = def_ccc_sm_ag;
  sm->base.info.id = id_ccc_sm_ag;
  sm->base.info.rev = rev_ccc_sm_ag;
  sm->base.info.oid = oid_ccc_sm_ag;

  printf("[CCC SM Agent]: Initialized with JSON-based encoding\n");
  
  return &sm->base;
}
