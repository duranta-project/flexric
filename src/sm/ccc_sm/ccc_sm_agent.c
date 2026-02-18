
#include "ccc_sm_agent.h"
#include "ccc_sm_id.h"
#include "ie/ccc_data_ie.h"
#include "../sm_proc_data.h"
#include "../../util/alg_ds/alg/defer.h"
#include "../../util/byte_array.h"
#include "../../util/time_now_us.h"

#include "enc/ccc_enc_plain.h"
#include "dec/ccc_dec_plain.h"

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
  ccc_enc_plain_t enc;
} sm_ccc_agent_t;

static
sm_ag_if_ans_subs_t on_subscription_ccc_sm_ag(sm_agent_t const* sm_agent, sm_subs_data_t const* data)
{
  assert(sm_agent != NULL);
  assert(data != NULL);
  (void)sm_agent;  // Unused - encoding type not needed for CCC plain

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
    // For now, we'll create a simple JSON payload
    
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
    
    // Set indication header
    ccc.ind.hdr.timestamp = time_now_us();
  }

  // Free memory allocated by RAN at read_ind
  defer({ 
    if (ccc.ind.msg.json_payload != NULL) {
      free(ccc.ind.msg.json_payload);
    }
  });

  // Encode indication header and message using plain encoder
  byte_array_t ba_hdr = ccc_enc_ind_hdr_plain(&ccc.ind.hdr);
  ret.data.ind_hdr = ba_hdr.buf;
  ret.data.len_hdr = ba_hdr.len;

  byte_array_t ba_msg = ccc_enc_ind_msg_plain(&ccc.ind.msg);
  ret.data.ind_msg = ba_msg.buf;
  ret.data.len_msg = ba_msg.len;

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
  
  // Decode control header and message using plain decoder
  ccc_ctrl_hdr_t ctrl_hdr = ccc_dec_ctrl_hdr_plain(data->len_hdr, data->ctrl_hdr);
  ccc_ctrl_msg_t ctrl_msg = ccc_dec_ctrl_msg_plain(data->len_msg, data->ctrl_msg);
  
  printf("[CCC SM Agent]: Control header received (type: %d)\n", ctrl_hdr.control_type);
  
  if (ctrl_msg.payload_len > 0 && ctrl_msg.json_payload != NULL) {
    printf("[CCC SM Agent]: Control message received: %.*s\n", (int)ctrl_msg.payload_len, ctrl_msg.json_payload);
    
    // Execute control action through I/O interface if available
    if (sm->base.io.write_ctrl != NULL) {
      ccc_ctrl_req_data_t req = {0};
      req.hdr = ctrl_hdr;
      req.msg = ctrl_msg;
      
      // Call the control function
      sm_ag_if_ans_t ans = sm->base.io.write_ctrl(&req);
      
      // Generate control outcome based on result
      if (ans.type == CTRL_OUTCOME_SM_AG_IF_ANS_V0 && ans.ctrl_out.type == CCC_AGENT_IF_CTRL_ANS_V0 && ans.ctrl_out.ccc.outcome == 0) {
        // Success - Generate E2SmCccControlOutcomeFormat1
        const char* outcome_json = 
          "{"
          "\"received_timestamp\": %ld,"
          "\"ran_configuration_structures_accepted_list\": ["
            "{"
              "\"ran_configuration_structure_name\": \"O_RU_Info\","
              "\"applied_timestamp\": %ld"
            "}"
          "],"
          "\"ran_configuration_structures_failed_list\": []"
          "}";
        
        char* formatted_outcome = calloc(512, sizeof(char));
        assert(formatted_outcome != NULL && "Memory exhausted");
        long timestamp = time_now_us();
        sprintf(formatted_outcome, outcome_json, timestamp, timestamp);
        
        out.len_out = strlen(formatted_outcome);
        out.ctrl_out = (uint8_t*)formatted_outcome;
      } else {
        // Failure case
        const char* failure_json = 
          "{"
          "\"received_timestamp\": %ld,"
          "\"ran_configuration_structures_accepted_list\": [],"
          "\"ran_configuration_structures_failed_list\": ["
            "{"
              "\"ran_configuration_structure_name\": \"Unknown\","
              "\"cause\": \"Configuration failed\""
            "}"
          "]"
          "}";
        
        char* formatted_failure = calloc(512, sizeof(char));
        assert(formatted_failure != NULL && "Memory exhausted");
        sprintf(formatted_failure, failure_json, time_now_us());
        
        out.len_out = strlen(formatted_failure);
        out.ctrl_out = (uint8_t*)formatted_failure;
      }
    } else {
      // Default success response when no I/O interface
      const char* default_outcome = 
        "{"
        "\"received_timestamp\": %ld,"
        "\"result\": \"success\""
        "}";
      
      char* formatted_default = calloc(256, sizeof(char));
      assert(formatted_default != NULL && "Memory exhausted");
      sprintf(formatted_default, default_outcome, time_now_us());
      
      out.len_out = strlen(formatted_default);
      out.ctrl_out = (uint8_t*)formatted_default;
    }
  } else {
    // No control message - generate minimal response
    const char* minimal_outcome = "{\"result\":\"no_action\"}";
    out.len_out = strlen(minimal_outcome);
    out.ctrl_out = calloc(out.len_out + 1, sizeof(uint8_t));
    assert(out.ctrl_out != NULL && "Memory exhausted");
    memcpy(out.ctrl_out, minimal_outcome, out.len_out);
  }
  
  // Cleanup decoded data
  if (ctrl_msg.json_payload != NULL) {
    free(ctrl_msg.json_payload);
  }
  
  return out;
}

static
sm_e2_setup_data_t on_e2_setup_ccc_sm_ag(sm_agent_t const* sm_agent)
{
  assert(sm_agent != NULL);
  (void)sm_agent;  // Unused - no I/O needed for E2 setup

  sm_e2_setup_data_t setup = {0};
  
  // Generate RAN function definition for CCC (JSON-based)
  const char* ran_func_def_json = 
    "{"
    "\"ranFunctionName\":\"CCC\","
    "\"ranFunctionId\":" TOSTRING(SM_CCC_ID) ","
    "\"ranFunctionDescription\":\"Cell Capacity and Coverage Service Model\","
    "\"ranFunctionRevision\":" TOSTRING(SM_CCC_REV) ","
    "\"encoding\":\"JSON\""
    "}";
  
  size_t def_len = strlen(ran_func_def_json);
  setup.ran_fun_def = calloc(def_len + 1, sizeof(uint8_t));
  assert(setup.ran_fun_def != NULL && "Memory exhausted");
  memcpy(setup.ran_fun_def, ran_func_def_json, def_len);
  setup.len_rfd = def_len;
  
  return setup;
}

static
sm_ric_service_update_data_t on_ric_service_update_ccc_sm_ag(sm_agent_t const* sm_agent)
{
  assert(sm_agent != NULL);
  (void)sm_agent;  // Unused - no I/O needed for service update

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
    sm_subs_data_t* subs = (sm_subs_data_t*)msg;
    if (subs->event_trigger != NULL) {
      free(subs->event_trigger);
    }
    if (subs->action_def != NULL) {
      free(subs->action_def);
    }
    free(subs);
  }
}

static
void free_ind_data_ccc_sm_ag(void* msg)
{
  if (msg != NULL) {
    sm_ag_if_rd_t* rd = (sm_ag_if_rd_t*)msg;
    if (rd->ind.ccc.msg.json_payload != NULL) {
      free(rd->ind.ccc.msg.json_payload);
    }
    free(rd);
  }
}

static
void free_ctrl_req_data_ccc_sm_ag(void* msg)
{
  if (msg != NULL) {
    sm_ctrl_req_data_t* ctrl = (sm_ctrl_req_data_t*)msg;
    if (ctrl->ctrl_hdr != NULL) {
      free(ctrl->ctrl_hdr);
    }
    if (ctrl->ctrl_msg != NULL) {
      free(ctrl->ctrl_msg);
    }
    free(ctrl);
  }
}

static
void free_ctrl_out_data_ccc_sm_ag(void* msg)
{
  if (msg != NULL) {
    sm_ctrl_out_data_t* out = (sm_ctrl_out_data_t*)msg;
    if (out->ctrl_out != NULL) {
      free(out->ctrl_out);
    }
    free(out);
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

  // Set I/O functions from RAN - convert from sm_io_ag_ran_t to sm_io_ag_sm_t
  sm->base.io.read_ind = io.read_ind_tbl[CCC_STATS_V0];
  sm->base.io.read_setup = io.read_setup_tbl[CCC_AGENT_IF_E2_SETUP_ANS_V0];
  
  // Write functions
  sm->base.io.write_ctrl = io.write_ctrl_tbl[CCC_CTRL_REQ_V0];
  sm->base.io.write_subs = NULL; // Used only for aperiodic subscription

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