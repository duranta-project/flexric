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
  // Nothing to free for basic struct
}

ccc_action_def_t cp_ccc_action_def(const ccc_action_def_t* src)
{
  assert(src != NULL);
  ccc_action_def_t dst = {0};
  dst.action_id = src->action_id;
  return dst;
}

bool eq_ccc_action_def(const ccc_action_def_t* m0, const ccc_action_def_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);
  return m0->action_id == m1->action_id;
}

//////////////////////////////////////
// Indication Header functions
/////////////////////////////////////

void free_ccc_ind_hdr(ccc_ind_hdr_t* src)
{
  assert(src != NULL);
  // Nothing to free for basic struct
}

ccc_ind_hdr_t cp_ccc_ind_hdr(const ccc_ind_hdr_t* src)
{
  assert(src != NULL);
  ccc_ind_hdr_t dst = {0};
  dst.timestamp = src->timestamp;
  return dst;
}

bool eq_ccc_ind_hdr(const ccc_ind_hdr_t* m0, const ccc_ind_hdr_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);
  return m0->timestamp == m1->timestamp;
}

//////////////////////////////////////
// Indication Message functions
/////////////////////////////////////

void free_ccc_ind_msg(ccc_ind_msg_t* src)
{
  assert(src != NULL);
  if(src->json_payload != NULL) {
    free(src->json_payload);
  }
}

ccc_ind_msg_t cp_ccc_ind_msg(const ccc_ind_msg_t* src)
{
  assert(src != NULL);
  ccc_ind_msg_t dst = {0};
  
  if(src->json_payload != NULL && src->payload_len > 0) {
    dst.json_payload = malloc(src->payload_len);
    memcpy(dst.json_payload, src->json_payload, src->payload_len);
    dst.payload_len = src->payload_len;
  }
  
  return dst;
}

bool eq_ccc_ind_msg(const ccc_ind_msg_t* m0, const ccc_ind_msg_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);
  
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
  // Nothing to free for basic struct
}

ccc_ctrl_hdr_t cp_ccc_ctrl_hdr(const ccc_ctrl_hdr_t* src)
{
  assert(src != NULL);
  ccc_ctrl_hdr_t dst = {0};
  dst.control_type = src->control_type;
  return dst;
}

bool eq_ccc_ctrl_hdr(const ccc_ctrl_hdr_t* m0, const ccc_ctrl_hdr_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);
  return m0->control_type == m1->control_type;
}

//////////////////////////////////////
// Control Message functions
/////////////////////////////////////

void free_ccc_ctrl_msg(ccc_ctrl_msg_t* src)
{
  assert(src != NULL);
  if(src->json_payload != NULL) {
    free(src->json_payload);
  }
}

ccc_ctrl_msg_t cp_ccc_ctrl_msg(const ccc_ctrl_msg_t* src)
{
  assert(src != NULL);
  ccc_ctrl_msg_t dst = {0};
  
  if(src->json_payload != NULL && src->payload_len > 0) {
    dst.json_payload = malloc(src->payload_len);
    memcpy(dst.json_payload, src->json_payload, src->payload_len);
    dst.payload_len = src->payload_len;
  }
  
  return dst;
}

bool eq_ccc_ctrl_msg(const ccc_ctrl_msg_t* m0, const ccc_ctrl_msg_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);
  
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
  // Nothing to free for basic struct
}

ccc_ctrl_out_t cp_ccc_ctrl_out(const ccc_ctrl_out_t* src)
{
  assert(src != NULL);
  ccc_ctrl_out_t dst = {0};
  dst.outcome = src->outcome;
  return dst;
}

bool eq_ccc_ctrl_out(const ccc_ctrl_out_t* m0, const ccc_ctrl_out_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);
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
