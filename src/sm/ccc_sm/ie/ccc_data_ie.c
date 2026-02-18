#include "ccc_data_ie.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

//////////////////////////////////////
// Helper function for string arrays
/////////////////////////////////////

static void free_string_array(char** strings, size_t len) {
  if (strings == NULL) return;
  for (size_t i = 0; i < len; i++) {
    free(strings[i]);
  }
  free(strings);
}

static char** copy_string_array(char** src, size_t len) {
  if (src == NULL || len == 0) return NULL;
  
  char** dst = calloc(len, sizeof(char*));
  assert(dst != NULL && "Memory exhausted");
  
  for (size_t i = 0; i < len; i++) {
    if (src[i] != NULL) {
      size_t str_len = strlen(src[i]) + 1;
      dst[i] = calloc(str_len, sizeof(char));
      assert(dst[i] != NULL && "Memory exhausted");
      memcpy(dst[i], src[i], str_len);
    }
  }
  
  return dst;
}

static bool eq_string_array(char** arr1, size_t len1, char** arr2, size_t len2) {
  if (len1 != len2) return false;
  if (arr1 == NULL && arr2 == NULL) return true;
  if (arr1 == NULL || arr2 == NULL) return false;
  
  for (size_t i = 0; i < len1; i++) {
    if (arr1[i] == NULL && arr2[i] == NULL) continue;
    if (arr1[i] == NULL || arr2[i] == NULL) return false;
    if (strcmp(arr1[i], arr2[i]) != 0) return false;
  }
  
  return true;
}

//////////////////////////////////////
// Items Element functions
/////////////////////////////////////

void free_ccc_items_element(ccc_items_element_t* src)
{
  assert(src != NULL);
  if(src->data != NULL) {
    free(src->data);
  }
}

ccc_items_element_t cp_ccc_items_element(const ccc_items_element_t* src)
{
  assert(src != NULL);
  ccc_items_element_t dst = {0};
  
  if(src->data != NULL && src->len > 0) {
    dst.data = malloc(src->len);
    assert(dst.data != NULL && "Memory exhausted");
    memcpy(dst.data, src->data, src->len);
    dst.len = src->len;
  }
  
  return dst;
}

bool eq_ccc_items_element(const ccc_items_element_t* m0, const ccc_items_element_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);
  
  if(m0->len != m1->len) return false;
  if(m0->data == NULL && m1->data == NULL) return true;
  if(m0->data == NULL || m1->data == NULL) return false;
  
  return memcmp(m0->data, m1->data, m0->len) == 0;
}

//////////////////////////////////////
// Event Trigger Format 1 functions
/////////////////////////////////////

void free_ccc_event_trigger_format1(ccc_event_trigger_format1_t* src)
{
  assert(src != NULL);
  free_ccc_items_element(&src->properties.list_of_node_level_configuration_structures_for_event_trigger);
  free_string_array(src->required, src->required_len);
}

ccc_event_trigger_format1_t cp_ccc_event_trigger_format1(const ccc_event_trigger_format1_t* src)
{
  assert(src != NULL);
  ccc_event_trigger_format1_t dst = {0};
  
  dst.type = src->type;
  dst.properties.list_of_node_level_configuration_structures_for_event_trigger = 
    cp_ccc_items_element(&src->properties.list_of_node_level_configuration_structures_for_event_trigger);
  dst.required = copy_string_array(src->required, src->required_len);
  dst.required_len = src->required_len;
  
  return dst;
}

bool eq_ccc_event_trigger_format1(const ccc_event_trigger_format1_t* m0, const ccc_event_trigger_format1_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);
  
  if (m0->type != m1->type) return false;
  if (!eq_ccc_items_element(&m0->properties.list_of_node_level_configuration_structures_for_event_trigger,
                           &m1->properties.list_of_node_level_configuration_structures_for_event_trigger)) {
    return false;
  }
  return eq_string_array(m0->required, m0->required_len, m1->required, m1->required_len);
}

//////////////////////////////////////
// Event Trigger Format 2 functions
/////////////////////////////////////

void free_ccc_event_trigger_format2(ccc_event_trigger_format2_t* src)
{
  assert(src != NULL);
  free_ccc_items_element(&src->properties.list_of_cell_level_configuration_structures_for_event_trigger);
  free_string_array(src->required, src->required_len);
}

ccc_event_trigger_format2_t cp_ccc_event_trigger_format2(const ccc_event_trigger_format2_t* src)
{
  assert(src != NULL);
  ccc_event_trigger_format2_t dst = {0};
  
  dst.type = src->type;
  dst.properties.list_of_cell_level_configuration_structures_for_event_trigger = 
    cp_ccc_items_element(&src->properties.list_of_cell_level_configuration_structures_for_event_trigger);
  dst.required = copy_string_array(src->required, src->required_len);
  dst.required_len = src->required_len;
  
  return dst;
}

bool eq_ccc_event_trigger_format2(const ccc_event_trigger_format2_t* m0, const ccc_event_trigger_format2_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);
  
  if (m0->type != m1->type) return false;
  if (!eq_ccc_items_element(&m0->properties.list_of_cell_level_configuration_structures_for_event_trigger,
                           &m1->properties.list_of_cell_level_configuration_structures_for_event_trigger)) {
    return false;
  }
  return eq_string_array(m0->required, m0->required_len, m1->required, m1->required_len);
}

//////////////////////////////////////
// Event Trigger Format 3 functions
/////////////////////////////////////

void free_ccc_event_trigger_format3(ccc_event_trigger_format3_t* src)
{
  assert(src != NULL);
  // period is a simple struct, nothing to free
  free_string_array(src->required, src->required_len);
}

ccc_event_trigger_format3_t cp_ccc_event_trigger_format3(const ccc_event_trigger_format3_t* src)
{
  assert(src != NULL);
  ccc_event_trigger_format3_t dst = {0};
  
  dst.type = src->type;
  dst.properties.period = src->properties.period;
  dst.required = copy_string_array(src->required, src->required_len);
  dst.required_len = src->required_len;
  
  return dst;
}

bool eq_ccc_event_trigger_format3(const ccc_event_trigger_format3_t* m0, const ccc_event_trigger_format3_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);
  
  if (m0->type != m1->type) return false;
  if (m0->properties.period.value != m1->properties.period.value) return false;
  return eq_string_array(m0->required, m0->required_len, m1->required, m1->required_len);
}

//////////////////////////////////////
// Event Trigger functions (main union)
/////////////////////////////////////

void free_ccc_event_trigger(ccc_event_trigger_t* src)
{
  assert(src != NULL);
  switch(src->format) {
    case FORMAT_1_E2SM_CCC_EV_TRIGGER_FORMAT:
      free_ccc_event_trigger_format1(&src->format1);
      break;
    case FORMAT_2_E2SM_CCC_EV_TRIGGER_FORMAT:
      free_ccc_event_trigger_format2(&src->format2);
      break;
    case FORMAT_3_E2SM_CCC_EV_TRIGGER_FORMAT:
      free_ccc_event_trigger_format3(&src->format3);
      break;
    default:
      assert(0 && "Unknown event trigger format");
  }
}

ccc_event_trigger_t cp_ccc_event_trigger(const ccc_event_trigger_t* src)
{
  assert(src != NULL);
  ccc_event_trigger_t dst = {0};
  dst.format = src->format;
  
  switch(src->format) {
    case FORMAT_1_E2SM_CCC_EV_TRIGGER_FORMAT:
      dst.format1 = cp_ccc_event_trigger_format1(&src->format1);
      break;
    case FORMAT_2_E2SM_CCC_EV_TRIGGER_FORMAT:
      dst.format2 = cp_ccc_event_trigger_format2(&src->format2);
      break;
    case FORMAT_3_E2SM_CCC_EV_TRIGGER_FORMAT:
      dst.format3 = cp_ccc_event_trigger_format3(&src->format3);
      break;
    default:
      assert(0 && "Unknown event trigger format");
  }
  
  return dst;
}

bool eq_ccc_event_trigger(const ccc_event_trigger_t* m0, const ccc_event_trigger_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);
  
  if (m0->format != m1->format) return false;
  
  switch(m0->format) {
    case FORMAT_1_E2SM_CCC_EV_TRIGGER_FORMAT:
      return eq_ccc_event_trigger_format1(&m0->format1, &m1->format1);
    case FORMAT_2_E2SM_CCC_EV_TRIGGER_FORMAT:
      return eq_ccc_event_trigger_format2(&m0->format2, &m1->format2);
    case FORMAT_3_E2SM_CCC_EV_TRIGGER_FORMAT:
      return eq_ccc_event_trigger_format3(&m0->format3, &m1->format3);
    default:
      assert(0 && "Unknown event trigger format");
      return false;
  }
}

//////////////////////////////////////
// Action Definition functions
/////////////////////////////////////

void free_ccc_action_def(ccc_action_def_t* src)
{
  assert(src != NULL);
  
  if(src->format == FORMAT_1_E2SM_CCC_ACTION_DEF) {
    free_ccc_items_element(&src->format1.list_of_node_level_ran_configuration_structures_for_adf);
  } else if(src->format == FORMAT_2_E2SM_CCC_ACTION_DEF) {
    free_ccc_items_element(&src->format2.list_of_cell_configurations_to_be_reported_for_adf);
  }
}

ccc_action_def_t cp_ccc_action_def(const ccc_action_def_t* src)
{
  assert(src != NULL);
  ccc_action_def_t dst = {0};
  
  dst.format = src->format;
  
  if(src->format == FORMAT_1_E2SM_CCC_ACTION_DEF) {
    dst.format1.list_of_node_level_ran_configuration_structures_for_adf = 
      cp_ccc_items_element(&src->format1.list_of_node_level_ran_configuration_structures_for_adf);
  } else if(src->format == FORMAT_2_E2SM_CCC_ACTION_DEF) {
    dst.format2.list_of_cell_configurations_to_be_reported_for_adf = 
      cp_ccc_items_element(&src->format2.list_of_cell_configurations_to_be_reported_for_adf);
  } else {
    dst.action_id = src->action_id;
  }
  
  return dst;
}

bool eq_ccc_action_def(const ccc_action_def_t* m0, const ccc_action_def_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);
  
  if(m0->format != m1->format) return false;
  
  if(m0->format == FORMAT_1_E2SM_CCC_ACTION_DEF) {
    return eq_ccc_items_element(&m0->format1.list_of_node_level_ran_configuration_structures_for_adf,
                                &m1->format1.list_of_node_level_ran_configuration_structures_for_adf);
  } else if(m0->format == FORMAT_2_E2SM_CCC_ACTION_DEF) {
    return eq_ccc_items_element(&m0->format2.list_of_cell_configurations_to_be_reported_for_adf,
                                &m1->format2.list_of_cell_configurations_to_be_reported_for_adf);
  }
  
  return m0->action_id == m1->action_id;
}

//////////////////////////////////////
// Indication Header functions
/////////////////////////////////////

void free_ccc_ind_hdr(ccc_ind_hdr_t* src)
{
  assert(src != NULL);
  
  if(src->format == FORMAT_1_E2SM_CCC_IND_HDR) {
    if(src->format1.indication_reason != NULL) {
      free(src->format1.indication_reason);
    }
  }
}

ccc_ind_hdr_t cp_ccc_ind_hdr(const ccc_ind_hdr_t* src)
{
  assert(src != NULL);
  ccc_ind_hdr_t dst = {0};
  
  dst.format = src->format;
  
  if(src->format == FORMAT_1_E2SM_CCC_IND_HDR) {
    if(src->format1.indication_reason != NULL) {
      dst.format1.indication_reason = strdup(src->format1.indication_reason);
      assert(dst.format1.indication_reason != NULL);
    }
    dst.format1.event_time = src->format1.event_time;
  } else {
    dst.timestamp = src->timestamp;
  }
  
  return dst;
}

bool eq_ccc_ind_hdr(const ccc_ind_hdr_t* m0, const ccc_ind_hdr_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);
  
  if(m0->format != m1->format) return false;
  
  if(m0->format == FORMAT_1_E2SM_CCC_IND_HDR) {
    bool reason_eq = true;
    if(m0->format1.indication_reason != NULL && m1->format1.indication_reason != NULL) {
      reason_eq = strcmp(m0->format1.indication_reason, m1->format1.indication_reason) == 0;
    } else {
      reason_eq = m0->format1.indication_reason == m1->format1.indication_reason;
    }
    return reason_eq && m0->format1.event_time == m1->format1.event_time;
  }
  
  return m0->timestamp == m1->timestamp;
}

//////////////////////////////////////
// Indication Message functions
/////////////////////////////////////

void free_ccc_ind_msg(ccc_ind_msg_t* src)
{
  assert(src != NULL);
  
  if(src->format == FORMAT_1_E2SM_CCC_IND_MSG) {
    free_ccc_items_element(&src->format1.list_of_configuration_structures_reported);
  } else if(src->format == FORMAT_2_E2SM_CCC_IND_MSG) {
    free_ccc_items_element(&src->format2.list_of_cells_reported);
  } else {
    if(src->json_payload != NULL) {
      free(src->json_payload);
    }
  }
}

ccc_ind_msg_t cp_ccc_ind_msg(const ccc_ind_msg_t* src)
{
  assert(src != NULL);
  ccc_ind_msg_t dst = {0};
  
  dst.format = src->format;
  
  if(src->format == FORMAT_1_E2SM_CCC_IND_MSG) {
    dst.format1.list_of_configuration_structures_reported = 
      cp_ccc_items_element(&src->format1.list_of_configuration_structures_reported);
  } else if(src->format == FORMAT_2_E2SM_CCC_IND_MSG) {
    dst.format2.list_of_cells_reported = 
      cp_ccc_items_element(&src->format2.list_of_cells_reported);
  } else {
    if(src->json_payload != NULL && src->payload_len > 0) {
      dst.json_payload = malloc(src->payload_len);
      memcpy(dst.json_payload, src->json_payload, src->payload_len);
      dst.payload_len = src->payload_len;
    }
  }
  
  return dst;
}

bool eq_ccc_ind_msg(const ccc_ind_msg_t* m0, const ccc_ind_msg_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);
  
  if(m0->format != m1->format) return false;
  
  if(m0->format == FORMAT_1_E2SM_CCC_IND_MSG) {
    return eq_ccc_items_element(&m0->format1.list_of_configuration_structures_reported,
                                &m1->format1.list_of_configuration_structures_reported);
  } else if(m0->format == FORMAT_2_E2SM_CCC_IND_MSG) {
    return eq_ccc_items_element(&m0->format2.list_of_cells_reported,
                                &m1->format2.list_of_cells_reported);
  }
  
  if(m0->payload_len != m1->payload_len)
    return false;
    
  if(m0->json_payload == NULL && m1->json_payload == NULL)
    return true;
    
  if(m0->json_payload == NULL || m1->json_payload == NULL)
    return false;
    
  return memcmp(m0->json_payload, m1->json_payload, m0->payload_len) == 0;
}

//////////////////////////////////////
// Control Header functions
/////////////////////////////////////

void free_ccc_ctrl_hdr(ccc_ctrl_hdr_t* src)
{
  assert(src != NULL);
  // Format 1 has no dynamic memory
}

ccc_ctrl_hdr_t cp_ccc_ctrl_hdr(const ccc_ctrl_hdr_t* src)
{
  assert(src != NULL);
  ccc_ctrl_hdr_t dst = {0};
  
  dst.format = src->format;
  
  if(src->format == FORMAT_1_E2SM_CCC_CTRL_HDR) {
    dst.format1.ric_style_type = src->format1.ric_style_type;
  } else {
    dst.control_type = src->control_type;
  }
  
  return dst;
}

bool eq_ccc_ctrl_hdr(const ccc_ctrl_hdr_t* m0, const ccc_ctrl_hdr_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);
  
  if(m0->format != m1->format) return false;
  
  if(m0->format == FORMAT_1_E2SM_CCC_CTRL_HDR) {
    return m0->format1.ric_style_type == m1->format1.ric_style_type;
  }
  
  return m0->control_type == m1->control_type;
}

//////////////////////////////////////
// Control Message functions
/////////////////////////////////////

void free_ccc_ctrl_msg(ccc_ctrl_msg_t* src)
{
  assert(src != NULL);
  
  if(src->format == FORMAT_1_E2SM_CCC_CTRL_MSG) {
    free_ccc_items_element(&src->format1.list_of_configuration_structures);
  } else if(src->format == FORMAT_2_E2SM_CCC_CTRL_MSG) {
    free_ccc_items_element(&src->format2.list_of_cells_controlled);
  } else {
    if(src->json_payload != NULL) {
      free(src->json_payload);
    }
  }
}

ccc_ctrl_msg_t cp_ccc_ctrl_msg(const ccc_ctrl_msg_t* src)
{
  assert(src != NULL);
  ccc_ctrl_msg_t dst = {0};
  
  dst.format = src->format;
  
  if(src->format == FORMAT_1_E2SM_CCC_CTRL_MSG) {
    dst.format1.list_of_configuration_structures = 
      cp_ccc_items_element(&src->format1.list_of_configuration_structures);
  } else if(src->format == FORMAT_2_E2SM_CCC_CTRL_MSG) {
    dst.format2.list_of_cells_controlled = 
      cp_ccc_items_element(&src->format2.list_of_cells_controlled);
  } else {
    if(src->json_payload != NULL && src->payload_len > 0) {
      dst.json_payload = malloc(src->payload_len);
      memcpy(dst.json_payload, src->json_payload, src->payload_len);
      dst.payload_len = src->payload_len;
    }
  }
  
  return dst;
}

bool eq_ccc_ctrl_msg(const ccc_ctrl_msg_t* m0, const ccc_ctrl_msg_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);
  
  if(m0->format != m1->format) return false;
  
  if(m0->format == FORMAT_1_E2SM_CCC_CTRL_MSG) {
    return eq_ccc_items_element(&m0->format1.list_of_configuration_structures,
                                &m1->format1.list_of_configuration_structures);
  } else if(m0->format == FORMAT_2_E2SM_CCC_CTRL_MSG) {
    return eq_ccc_items_element(&m0->format2.list_of_cells_controlled,
                                &m1->format2.list_of_cells_controlled);
  }
  
  if(m0->payload_len != m1->payload_len)
    return false;
    
  if(m0->json_payload == NULL && m1->json_payload == NULL)
    return true;
    
  if(m0->json_payload == NULL || m1->json_payload == NULL)
    return false;
    
  return memcmp(m0->json_payload, m1->json_payload, m0->payload_len) == 0;
}

//////////////////////////////////////
// Control Outcome functions
/////////////////////////////////////

void free_ccc_ctrl_out(ccc_ctrl_out_t* src)
{
  assert(src != NULL);
  
  if(src->format == FORMAT_1_E2SM_CCC_CTRL_OUT) {
    free_ccc_items_element(&src->format1.ran_configuration_structures_accepted_list);
    free_ccc_items_element(&src->format1.ran_configuration_structures_failed_list);
  } else if(src->format == FORMAT_2_E2SM_CCC_CTRL_OUT) {
    free_ccc_items_element(&src->format2.list_of_cells_for_control_outcome);
  }
}

ccc_ctrl_out_t cp_ccc_ctrl_out(const ccc_ctrl_out_t* src)
{
  assert(src != NULL);
  ccc_ctrl_out_t dst = {0};
  
  dst.format = src->format;
  
  if(src->format == FORMAT_1_E2SM_CCC_CTRL_OUT) {
    dst.format1.received_timestamp = src->format1.received_timestamp;
    dst.format1.ran_configuration_structures_accepted_list = 
      cp_ccc_items_element(&src->format1.ran_configuration_structures_accepted_list);
    dst.format1.ran_configuration_structures_failed_list = 
      cp_ccc_items_element(&src->format1.ran_configuration_structures_failed_list);
  } else if(src->format == FORMAT_2_E2SM_CCC_CTRL_OUT) {
    dst.format2.received_timestamp = src->format2.received_timestamp;
    dst.format2.list_of_cells_for_control_outcome = 
      cp_ccc_items_element(&src->format2.list_of_cells_for_control_outcome);
  } else {
    dst.outcome = src->outcome;
  }
  
  return dst;
}

bool eq_ccc_ctrl_out(const ccc_ctrl_out_t* m0, const ccc_ctrl_out_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);
  
  if(m0->format != m1->format) return false;
  
  if(m0->format == FORMAT_1_E2SM_CCC_CTRL_OUT) {
    return m0->format1.received_timestamp == m1->format1.received_timestamp &&
           eq_ccc_items_element(&m0->format1.ran_configuration_structures_accepted_list,
                                &m1->format1.ran_configuration_structures_accepted_list) &&
           eq_ccc_items_element(&m0->format1.ran_configuration_structures_failed_list,
                                &m1->format1.ran_configuration_structures_failed_list);
  } else if(m0->format == FORMAT_2_E2SM_CCC_CTRL_OUT) {
    return m0->format2.received_timestamp == m1->format2.received_timestamp &&
           eq_ccc_items_element(&m0->format2.list_of_cells_for_control_outcome,
                                &m1->format2.list_of_cells_for_control_outcome);
  }
  
  return m0->outcome == m1->outcome;
}

//////////////////////////////////////
// Function Definition functions
/////////////////////////////////////

void free_ccc_func_def(ccc_func_def_t* src)
{
  assert(src != NULL);
  if(src->func_def != NULL) {
    free(src->func_def);
  }
}

ccc_func_def_t cp_ccc_func_def(const ccc_func_def_t* src)
{
  assert(src != NULL);
  ccc_func_def_t dst = {0};
  
  if(src->func_def != NULL && src->len > 0) {
    dst.func_def = malloc(src->len);
    memcpy(dst.func_def, src->func_def, src->len);
    dst.len = src->len;
  }
  
  return dst;
}

bool eq_ccc_func_def(const ccc_func_def_t* m0, const ccc_func_def_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);
  
  if(m0->len != m1->len)
    return false;
    
  if(m0->func_def == NULL && m1->func_def == NULL)
    return true;
    
  if(m0->func_def == NULL || m1->func_def == NULL)
    return false;
    
  return memcmp(m0->func_def, m1->func_def, m0->len) == 0;
}

//////////////////////////////////////
// Higher level struct functions
/////////////////////////////////////

void free_ccc_ind_data(ccc_ind_data_t* src)
{
  assert(src != NULL);
  free_ccc_ind_hdr(&src->hdr);
  free_ccc_ind_msg(&src->msg);
}

ccc_ind_data_t cp_ccc_ind_data(const ccc_ind_data_t* src)
{
  assert(src != NULL);
  ccc_ind_data_t dst = {0};
  dst.hdr = cp_ccc_ind_hdr(&src->hdr);
  dst.msg = cp_ccc_ind_msg(&src->msg);
  return dst;
}

bool eq_ccc_ind_data(const ccc_ind_data_t* m0, const ccc_ind_data_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);
  
  if(!eq_ccc_ind_hdr(&m0->hdr, &m1->hdr))
    return false;
    
  return eq_ccc_ind_msg(&m0->msg, &m1->msg);
}

void free_ccc_ctrl_req_data(ccc_ctrl_req_data_t* src)
{
  assert(src != NULL);
  free_ccc_ctrl_hdr(&src->hdr);
  free_ccc_ctrl_msg(&src->msg);
}

ccc_ctrl_req_data_t cp_ccc_ctrl_req_data(const ccc_ctrl_req_data_t* src)
{
  assert(src != NULL);
  ccc_ctrl_req_data_t dst = {0};
  dst.hdr = cp_ccc_ctrl_hdr(&src->hdr);
  dst.msg = cp_ccc_ctrl_msg(&src->msg);
  return dst;
}

bool eq_ccc_ctrl_req_data(const ccc_ctrl_req_data_t* m0, const ccc_ctrl_req_data_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);
  
  if(!eq_ccc_ctrl_hdr(&m0->hdr, &m1->hdr))
    return false;
    
  return eq_ccc_ctrl_msg(&m0->msg, &m1->msg);
}

void free_ccc_e2_setup_data(ccc_e2_setup_data_t* src)
{
  assert(src != NULL);
  free_ccc_func_def(&src->func_def);
}

void free_ccc_ric_service_update(ccc_ric_service_update_t* src)
{
  assert(src != NULL);
  free_ccc_func_def(&src->func_def);
}

void free_ccc_sub_data(ccc_sub_data_t* src)
{
  assert(src != NULL);
  free_ccc_event_trigger(&src->trigger);
  free_ccc_action_def(&src->action_def);
}

//////////////////////////////////////
// CGI functions
/////////////////////////////////////

void free_ccc_cgi(ccc_cgi_t* src)
{
  assert(src != NULL);
  // No dynamic memory in CGI
}

ccc_cgi_t cp_ccc_cgi(const ccc_cgi_t* src)
{
  assert(src != NULL);
  ccc_cgi_t dst = {0};
  dst.type = src->type;
  
  if(src->type == CCC_CGI_EUTRA) {
    memcpy(&dst.eutra_cgi, &src->eutra_cgi, sizeof(ccc_eutra_cgi_t));
  } else if(src->type == CCC_CGI_NR) {
    memcpy(&dst.nr_cgi, &src->nr_cgi, sizeof(ccc_nr_cgi_t));
  }
  
  return dst;
}

bool eq_ccc_cgi(const ccc_cgi_t* m0, const ccc_cgi_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);
  
  if(m0->type != m1->type) return false;
  
  if(m0->type == CCC_CGI_EUTRA) {
    return memcmp(&m0->eutra_cgi, &m1->eutra_cgi, sizeof(ccc_eutra_cgi_t)) == 0;
  } else if(m0->type == CCC_CGI_NR) {
    return memcmp(&m0->nr_cgi, &m1->nr_cgi, sizeof(ccc_nr_cgi_t)) == 0;
  }
  
  return false;
}

//////////////////////////////////////
// Energy Saving Capability functions
/////////////////////////////////////

void free_ccc_energy_saving_capability(ccc_energy_saving_capability_common_info_t* src)
{
  assert(src != NULL);
  // No dynamic memory
}

ccc_energy_saving_capability_common_info_t cp_ccc_energy_saving_capability(const ccc_energy_saving_capability_common_info_t* src)
{
  assert(src != NULL);
  ccc_energy_saving_capability_common_info_t dst = *src;
  return dst;
}

bool eq_ccc_energy_saving_capability(const ccc_energy_saving_capability_common_info_t* m0, const ccc_energy_saving_capability_common_info_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);
  
  return m0->st8_ready_message_supported == m1->st8_ready_message_supported &&
         m0->sleep_duration_extension_supported == m1->sleep_duration_extension_supported &&
         m0->emergency_wake_up_command_supported == m1->emergency_wake_up_command_supported;
}

//////////////////////////////////////
// RAN Configuration Structure functions
/////////////////////////////////////

void free_ccc_ran_configuration_structure(ccc_ran_configuration_structure_t* src)
{
  assert(src != NULL);
  
  if(src->ran_configuration_structure_name != NULL) {
    free(src->ran_configuration_structure_name);
  }
  if(src->values_of_attributes != NULL) {
    free(src->values_of_attributes);
  }
}

ccc_ran_configuration_structure_t cp_ccc_ran_configuration_structure(const ccc_ran_configuration_structure_t* src)
{
  assert(src != NULL);
  ccc_ran_configuration_structure_t dst = {0};
  
  dst.name_enum = src->name_enum;
  
  if(src->ran_configuration_structure_name != NULL) {
    dst.ran_configuration_structure_name = strdup(src->ran_configuration_structure_name);
    assert(dst.ran_configuration_structure_name != NULL);
  }
  
  if(src->values_of_attributes != NULL && src->values_len > 0) {
    dst.values_of_attributes = malloc(src->values_len);
    assert(dst.values_of_attributes != NULL);
    memcpy(dst.values_of_attributes, src->values_of_attributes, src->values_len);
    dst.values_len = src->values_len;
  }
  
  return dst;
}

bool eq_ccc_ran_configuration_structure(const ccc_ran_configuration_structure_t* m0, const ccc_ran_configuration_structure_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);
  
  if(m0->name_enum != m1->name_enum) return false;
  
  if(m0->ran_configuration_structure_name != NULL && m1->ran_configuration_structure_name != NULL) {
    if(strcmp(m0->ran_configuration_structure_name, m1->ran_configuration_structure_name) != 0) return false;
  } else if(m0->ran_configuration_structure_name != m1->ran_configuration_structure_name) {
    return false;
  }
  
  if(m0->values_len != m1->values_len) return false;
  
  if(m0->values_of_attributes != NULL && m1->values_of_attributes != NULL) {
    return memcmp(m0->values_of_attributes, m1->values_of_attributes, m0->values_len) == 0;
  }
  
  return m0->values_of_attributes == m1->values_of_attributes;
}

//////////////////////////////////////
// RAN Configuration Structure List functions
/////////////////////////////////////

void free_ccc_ran_configuration_structure_list(ccc_ran_configuration_structure_list_t* src)
{
  assert(src != NULL);
  
  if(src->items != NULL) {
    for(size_t i = 0; i < src->len; i++) {
      free_ccc_ran_configuration_structure(&src->items[i]);
    }
    free(src->items);
  }
}

ccc_ran_configuration_structure_list_t cp_ccc_ran_configuration_structure_list(const ccc_ran_configuration_structure_list_t* src)
{
  assert(src != NULL);
  ccc_ran_configuration_structure_list_t dst = {0};
  
  if(src->items != NULL && src->len > 0) {
    dst.items = calloc(src->len, sizeof(ccc_ran_configuration_structure_t));
    assert(dst.items != NULL);
    dst.len = src->len;
    
    for(size_t i = 0; i < src->len; i++) {
      dst.items[i] = cp_ccc_ran_configuration_structure(&src->items[i]);
    }
  }
  
  return dst;
}

bool eq_ccc_ran_configuration_structure_list(const ccc_ran_configuration_structure_list_t* m0, const ccc_ran_configuration_structure_list_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);
  
  if(m0->len != m1->len) return false;
  
  for(size_t i = 0; i < m0->len; i++) {
    if(!eq_ccc_ran_configuration_structure(&m0->items[i], &m1->items[i])) {
      return false;
    }
  }
  
  return true;
}

//////////////////////////////////////
// Cell Info functions
/////////////////////////////////////

void free_ccc_cell_info(ccc_cell_info_t* src)
{
  assert(src != NULL);
  
  free_ccc_cgi(&src->cgi);
  if(src->additional_info != NULL) {
    free(src->additional_info);
  }
}

ccc_cell_info_t cp_ccc_cell_info(const ccc_cell_info_t* src)
{
  assert(src != NULL);
  ccc_cell_info_t dst = {0};
  
  dst.cgi = cp_ccc_cgi(&src->cgi);
  dst.cell_local_id = src->cell_local_id;
  dst.admin_state = src->admin_state;
  
  if(src->additional_info != NULL && src->additional_info_len > 0) {
    dst.additional_info = malloc(src->additional_info_len);
    assert(dst.additional_info != NULL);
    memcpy(dst.additional_info, src->additional_info, src->additional_info_len);
    dst.additional_info_len = src->additional_info_len;
  }
  
  return dst;
}

bool eq_ccc_cell_info(const ccc_cell_info_t* m0, const ccc_cell_info_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);
  
  if(!eq_ccc_cgi(&m0->cgi, &m1->cgi)) return false;
  if(m0->cell_local_id != m1->cell_local_id) return false;
  if(m0->admin_state != m1->admin_state) return false;
  if(m0->additional_info_len != m1->additional_info_len) return false;
  
  if(m0->additional_info != NULL && m1->additional_info != NULL) {
    return memcmp(m0->additional_info, m1->additional_info, m0->additional_info_len) == 0;
  }
  
  return m0->additional_info == m1->additional_info;
}

//////////////////////////////////////
// Cell List functions
/////////////////////////////////////

void free_ccc_cell_list(ccc_cell_list_t* src)
{
  assert(src != NULL);
  
  if(src->cells != NULL) {
    for(size_t i = 0; i < src->len; i++) {
      free_ccc_cell_info(&src->cells[i]);
    }
    free(src->cells);
  }
}

ccc_cell_list_t cp_ccc_cell_list(const ccc_cell_list_t* src)
{
  assert(src != NULL);
  ccc_cell_list_t dst = {0};
  
  if(src->cells != NULL && src->len > 0) {
    dst.cells = calloc(src->len, sizeof(ccc_cell_info_t));
    assert(dst.cells != NULL);
    dst.len = src->len;
    
    for(size_t i = 0; i < src->len; i++) {
      dst.cells[i] = cp_ccc_cell_info(&src->cells[i]);
    }
  }
  
  return dst;
}

bool eq_ccc_cell_list(const ccc_cell_list_t* m0, const ccc_cell_list_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);
  
  if(m0->len != m1->len) return false;
  
  for(size_t i = 0; i < m0->len; i++) {
    if(!eq_ccc_cell_info(&m0->cells[i], &m1->cells[i])) {
      return false;
    }
  }
  
  return true;
}

//////////////////////////////////////
// BWP Configuration functions
/////////////////////////////////////

void free_ccc_bwp_config(ccc_bwp_config_t* src)
{
  assert(src != NULL);
  // No dynamic memory
}

ccc_bwp_config_t cp_ccc_bwp_config(const ccc_bwp_config_t* src)
{
  assert(src != NULL);
  ccc_bwp_config_t dst = *src;
  return dst;
}

bool eq_ccc_bwp_config(const ccc_bwp_config_t* m0, const ccc_bwp_config_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);
  
  return m0->bwp_context == m1->bwp_context &&
         m0->is_initial_bwp == m1->is_initial_bwp &&
         m0->sub_carrier_spacing == m1->sub_carrier_spacing &&
         m0->cyclic_prefix == m1->cyclic_prefix &&
         m0->start_rb == m1->start_rb &&
         m0->number_of_rbs == m1->number_of_rbs;
}

//////////////////////////////////////
// DTX/DRX Configuration functions
/////////////////////////////////////

void free_ccc_cell_dtxdrx_config(ccc_cell_dtxdrx_config_t* src)
{
  assert(src != NULL);
  // No dynamic memory
}

ccc_cell_dtxdrx_config_t cp_ccc_cell_dtxdrx_config(const ccc_cell_dtxdrx_config_t* src)
{
  assert(src != NULL);
  ccc_cell_dtxdrx_config_t dst = *src;
  return dst;
}

bool eq_ccc_cell_dtxdrx_config(const ccc_cell_dtxdrx_config_t* m0, const ccc_cell_dtxdrx_config_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);
  
  return m0->on_duration_timer == m1->on_duration_timer &&
         m0->cycle_start_offset == m1->cycle_start_offset &&
         m0->slot_offset == m1->slot_offset &&
         m0->config_type == m1->config_type &&
         m0->activation_status == m1->activation_status &&
         m0->l1_activation == m1->l1_activation;
}

//////////////////////////////////////
// CES Management Function
/////////////////////////////////////

void free_ccc_ces_management_function(ccc_ces_management_function_t* src)
{
  assert(src != NULL);
  // No dynamic memory
}

ccc_ces_management_function_t cp_ccc_ces_management_function(const ccc_ces_management_function_t* src)
{
  assert(src != NULL);
  ccc_ces_management_function_t dst = *src;
  return dst;
}

bool eq_ccc_ces_management_function(const ccc_ces_management_function_t* m0, const ccc_ces_management_function_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);
  
  return m0->ces_switch == m1->ces_switch &&
         m0->energy_saving_state == m1->energy_saving_state &&
         m0->energy_saving_control == m1->energy_saving_control;
}

//////////////////////////////////////
// Configuration Structure Accepted/Failed functions
/////////////////////////////////////

void free_ccc_config_structure_accepted(ccc_config_structure_accepted_t* src)
{
  assert(src != NULL);
  if(src->ran_configuration_structure_name != NULL) {
    free(src->ran_configuration_structure_name);
  }
}

ccc_config_structure_accepted_t cp_ccc_config_structure_accepted(const ccc_config_structure_accepted_t* src)
{
  assert(src != NULL);
  ccc_config_structure_accepted_t dst = {0};
  
  if(src->ran_configuration_structure_name != NULL) {
    dst.ran_configuration_structure_name = strdup(src->ran_configuration_structure_name);
    assert(dst.ran_configuration_structure_name != NULL);
  }
  dst.applied_timestamp = src->applied_timestamp;
  
  return dst;
}

bool eq_ccc_config_structure_accepted(const ccc_config_structure_accepted_t* m0, const ccc_config_structure_accepted_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);
  
  if(m0->applied_timestamp != m1->applied_timestamp) return false;
  
  if(m0->ran_configuration_structure_name != NULL && m1->ran_configuration_structure_name != NULL) {
    return strcmp(m0->ran_configuration_structure_name, m1->ran_configuration_structure_name) == 0;
  }
  
  return m0->ran_configuration_structure_name == m1->ran_configuration_structure_name;
}

void free_ccc_config_structure_failed(ccc_config_structure_failed_t* src)
{
  assert(src != NULL);
  
  if(src->ran_configuration_structure_name != NULL) {
    free(src->ran_configuration_structure_name);
  }
  if(src->cause != NULL) {
    free(src->cause);
  }
}

ccc_config_structure_failed_t cp_ccc_config_structure_failed(const ccc_config_structure_failed_t* src)
{
  assert(src != NULL);
  ccc_config_structure_failed_t dst = {0};
  
  if(src->ran_configuration_structure_name != NULL) {
    dst.ran_configuration_structure_name = strdup(src->ran_configuration_structure_name);
    assert(dst.ran_configuration_structure_name != NULL);
  }
  
  if(src->cause != NULL) {
    dst.cause = strdup(src->cause);
    assert(dst.cause != NULL);
  }
  
  return dst;
}

bool eq_ccc_config_structure_failed(const ccc_config_structure_failed_t* m0, const ccc_config_structure_failed_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);
  
  bool name_eq = true;
  if(m0->ran_configuration_structure_name != NULL && m1->ran_configuration_structure_name != NULL) {
    name_eq = strcmp(m0->ran_configuration_structure_name, m1->ran_configuration_structure_name) == 0;
  } else {
    name_eq = m0->ran_configuration_structure_name == m1->ran_configuration_structure_name;
  }
  
  bool cause_eq = true;
  if(m0->cause != NULL && m1->cause != NULL) {
    cause_eq = strcmp(m0->cause, m1->cause) == 0;
  } else {
    cause_eq = m0->cause == m1->cause;
  }
  
  return name_eq && cause_eq;
}

//////////////////////////////////////
// Query Definition functions
/////////////////////////////////////

void free_ccc_query_def(ccc_query_def_t* src)
{
  assert(src != NULL);
  
  if(src->format == FORMAT_1_E2SM_CCC_QUERY_DEF) {
    free_ccc_items_element(&src->format1.list_of_nodelevel_ran_configuration_structures_for_query);
  } else if(src->format == FORMAT_2_E2SM_CCC_QUERY_DEF) {
    free_ccc_items_element(&src->format2.list_of_cells_to_be_queried_for_query);
  }
}

ccc_query_def_t cp_ccc_query_def(const ccc_query_def_t* src)
{
  assert(src != NULL);
  ccc_query_def_t dst = {0};
  
  dst.format = src->format;
  
  if(src->format == FORMAT_1_E2SM_CCC_QUERY_DEF) {
    dst.format1.list_of_nodelevel_ran_configuration_structures_for_query = 
      cp_ccc_items_element(&src->format1.list_of_nodelevel_ran_configuration_structures_for_query);
  } else if(src->format == FORMAT_2_E2SM_CCC_QUERY_DEF) {
    dst.format2.list_of_cells_to_be_queried_for_query = 
      cp_ccc_items_element(&src->format2.list_of_cells_to_be_queried_for_query);
  }
  
  return dst;
}

bool eq_ccc_query_def(const ccc_query_def_t* m0, const ccc_query_def_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);
  
  if(m0->format != m1->format) return false;
  
  if(m0->format == FORMAT_1_E2SM_CCC_QUERY_DEF) {
    return eq_ccc_items_element(&m0->format1.list_of_nodelevel_ran_configuration_structures_for_query,
                                &m1->format1.list_of_nodelevel_ran_configuration_structures_for_query);
  } else if(m0->format == FORMAT_2_E2SM_CCC_QUERY_DEF) {
    return eq_ccc_items_element(&m0->format2.list_of_cells_to_be_queried_for_query,
                                &m1->format2.list_of_cells_to_be_queried_for_query);
  }
  
  return false;
}

//////////////////////////////////////
// Query Outcome functions
/////////////////////////////////////

void free_ccc_query_out(ccc_query_out_t* src)
{
  assert(src != NULL);
  
  if(src->format == FORMAT_1_E2SM_CCC_QUERY_OUT) {
    free_ccc_items_element(&src->format1.list_of_configuration_structures_to_be_reported_for_query);
  } else if(src->format == FORMAT_2_E2SM_CCC_QUERY_OUT) {
    free_ccc_items_element(&src->format2.list_of_cells_to_be_reported_for_query);
  }
}

ccc_query_out_t cp_ccc_query_out(const ccc_query_out_t* src)
{
  assert(src != NULL);
  ccc_query_out_t dst = {0};
  
  dst.format = src->format;
  
  if(src->format == FORMAT_1_E2SM_CCC_QUERY_OUT) {
    dst.format1.list_of_configuration_structures_to_be_reported_for_query = 
      cp_ccc_items_element(&src->format1.list_of_configuration_structures_to_be_reported_for_query);
  } else if(src->format == FORMAT_2_E2SM_CCC_QUERY_OUT) {
    dst.format2.list_of_cells_to_be_reported_for_query = 
      cp_ccc_items_element(&src->format2.list_of_cells_to_be_reported_for_query);
  }
  
  return dst;
}

bool eq_ccc_query_out(const ccc_query_out_t* m0, const ccc_query_out_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);
  
  if(m0->format != m1->format) return false;
  
  if(m0->format == FORMAT_1_E2SM_CCC_QUERY_OUT) {
    return eq_ccc_items_element(&m0->format1.list_of_configuration_structures_to_be_reported_for_query,
                                &m1->format1.list_of_configuration_structures_to_be_reported_for_query);
  } else if(m0->format == FORMAT_2_E2SM_CCC_QUERY_OUT) {
    return eq_ccc_items_element(&m0->format2.list_of_cells_to_be_reported_for_query,
                                &m1->format2.list_of_cells_to_be_reported_for_query);
  }
  
  return false;
}
