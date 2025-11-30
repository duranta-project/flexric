#ifndef CCC_DATA_INFORMATION_ELEMENTS_H
#define CCC_DATA_INFORMATION_ELEMENTS_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Information Elements (IE) for CCC Service Model
 * Based on your C++ structures in e2sm_ccc.hpp
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

//////////////////////////////////////
// RIC Event Trigger Definition
/////////////////////////////////////

typedef enum{
  FORMAT_1_E2SM_CCC_EV_TRIGGER_FORMAT = 1,//9.2.1.1.1
  FORMAT_2_E2SM_CCC_EV_TRIGGER_FORMAT = 2,//9.2.1.1.2
  FORMAT_3_E2SM_CCC_EV_TRIGGER_FORMAT = 3,//9.2.1.1.3

  END_E2SM_CCC_EV_TRIGGER_FORMAT
} e2sm_ccc_ev_trigger_format_e; 

// Forward declarations for complex types
typedef struct {
  char* data;
  size_t len;
} ccc_items_element_t;

typedef enum {
  CCC_EUTRA_CELL_IDENTITY_TYPE_OBJECT = 0,
  CCC_EUTRA_CELL_IDENTITY_TYPE_STRING = 1
} ccc_eutra_cell_identity_type_e;

typedef struct {
  int64_t value;  // Simplified - could be more complex
} ccc_eutra_cell_identity_t;

// Event Trigger Format 1 - Node Level Configuration
typedef struct {
  ccc_items_element_t list_of_node_level_configuration_structures_for_event_trigger;
} ccc_event_trigger_format1_properties_t;

typedef struct {
  ccc_eutra_cell_identity_type_e type;
  ccc_event_trigger_format1_properties_t properties;
  char** required;      // Array of strings
  size_t required_len;  // Number of required elements
} ccc_event_trigger_format1_t;

// Event Trigger Format 2 - Cell Level Configuration  
typedef struct {
  ccc_items_element_t list_of_cell_level_configuration_structures_for_event_trigger;
} ccc_event_trigger_format2_properties_t;

typedef struct {
  ccc_eutra_cell_identity_type_e type;
  ccc_event_trigger_format2_properties_t properties;
  char** required;
  size_t required_len;
} ccc_event_trigger_format2_t;

// Event Trigger Format 3 - Periodic
typedef struct {
  ccc_eutra_cell_identity_t period;
} ccc_event_trigger_format3_properties_t;

typedef struct {
  ccc_eutra_cell_identity_type_e type;
  ccc_event_trigger_format3_properties_t properties;
  char** required;
  size_t required_len;
} ccc_event_trigger_format3_t;

// Main Event Trigger Union
typedef struct {
  e2sm_ccc_ev_trigger_format_e format;
  union {
    ccc_event_trigger_format1_t format1;
    ccc_event_trigger_format2_t format2;
    ccc_event_trigger_format3_t format3;
  };
} ccc_event_trigger_t;

// Function declarations
void free_ccc_items_element(ccc_items_element_t* src);
ccc_items_element_t cp_ccc_items_element(const ccc_items_element_t* src);
bool eq_ccc_items_element(const ccc_items_element_t* m0, const ccc_items_element_t* m1);

void free_ccc_event_trigger_format1(ccc_event_trigger_format1_t* src);
ccc_event_trigger_format1_t cp_ccc_event_trigger_format1(const ccc_event_trigger_format1_t* src);
bool eq_ccc_event_trigger_format1(const ccc_event_trigger_format1_t* m0, const ccc_event_trigger_format1_t* m1);

void free_ccc_event_trigger_format2(ccc_event_trigger_format2_t* src);
ccc_event_trigger_format2_t cp_ccc_event_trigger_format2(const ccc_event_trigger_format2_t* src);
bool eq_ccc_event_trigger_format2(const ccc_event_trigger_format2_t* m0, const ccc_event_trigger_format2_t* m1);

void free_ccc_event_trigger_format3(ccc_event_trigger_format3_t* src);
ccc_event_trigger_format3_t cp_ccc_event_trigger_format3(const ccc_event_trigger_format3_t* src);
bool eq_ccc_event_trigger_format3(const ccc_event_trigger_format3_t* m0, const ccc_event_trigger_format3_t* m1);

void free_ccc_event_trigger(ccc_event_trigger_t* src);
ccc_event_trigger_t cp_ccc_event_trigger(const ccc_event_trigger_t* src);
bool eq_ccc_event_trigger(const ccc_event_trigger_t* m0, const ccc_event_trigger_t* m1);

//////////////////////////////////////
// RIC Action Definition  
/////////////////////////////////////

typedef struct {
  // Action definition parameters
  uint32_t action_id;
} ccc_action_def_t;

void free_ccc_action_def(ccc_action_def_t* src);
ccc_action_def_t cp_ccc_action_def(const ccc_action_def_t* src);
bool eq_ccc_action_def(const ccc_action_def_t* m0, const ccc_action_def_t* m1);

//////////////////////////////////////
// RIC Indication Header
/////////////////////////////////////

typedef struct {
  uint32_t timestamp;
} ccc_ind_hdr_t;

void free_ccc_ind_hdr(ccc_ind_hdr_t* src);
ccc_ind_hdr_t cp_ccc_ind_hdr(const ccc_ind_hdr_t* src);
bool eq_ccc_ind_hdr(const ccc_ind_hdr_t* m0, const ccc_ind_hdr_t* m1);

//////////////////////////////////////
// RIC Indication Message
/////////////////////////////////////

typedef struct {
  // This will contain your CCC specific data
  char* json_payload;  // For now, using JSON payload
  size_t payload_len;
} ccc_ind_msg_t;

void free_ccc_ind_msg(ccc_ind_msg_t* src);
ccc_ind_msg_t cp_ccc_ind_msg(const ccc_ind_msg_t* src);
bool eq_ccc_ind_msg(const ccc_ind_msg_t* m0, const ccc_ind_msg_t* m1);

//////////////////////////////////////
// RIC Control Header
/////////////////////////////////////

typedef struct {
  uint32_t control_type;
} ccc_ctrl_hdr_t;

void free_ccc_ctrl_hdr(ccc_ctrl_hdr_t* src);
ccc_ctrl_hdr_t cp_ccc_ctrl_hdr(const ccc_ctrl_hdr_t* src);
bool eq_ccc_ctrl_hdr(const ccc_ctrl_hdr_t* m0, const ccc_ctrl_hdr_t* m1);

//////////////////////////////////////
// RIC Control Message
/////////////////////////////////////

typedef struct {
  // This will contain your CCC control data
  char* json_payload;  // For now, using JSON payload
  size_t payload_len;
} ccc_ctrl_msg_t;

void free_ccc_ctrl_msg(ccc_ctrl_msg_t* src);
ccc_ctrl_msg_t cp_ccc_ctrl_msg(const ccc_ctrl_msg_t* src);
bool eq_ccc_ctrl_msg(const ccc_ctrl_msg_t* m0, const ccc_ctrl_msg_t* m1);

//////////////////////////////////////
// RIC Control Outcome
/////////////////////////////////////

typedef struct {
  uint32_t outcome;
} ccc_ctrl_out_t;

void free_ccc_ctrl_out(ccc_ctrl_out_t* src);
ccc_ctrl_out_t cp_ccc_ctrl_out(const ccc_ctrl_out_t* src);
bool eq_ccc_ctrl_out(const ccc_ctrl_out_t* m0, const ccc_ctrl_out_t* m1);

//////////////////////////////////////
// Function Definition
/////////////////////////////////////

typedef struct {
  char* func_def;
  size_t len;
} ccc_func_def_t;

void free_ccc_func_def(ccc_func_def_t* src);
ccc_func_def_t cp_ccc_func_def(const ccc_func_def_t* src);
bool eq_ccc_func_def(const ccc_func_def_t* m0, const ccc_func_def_t* m1);

//////////////////////////////////////
// Higher level structs
/////////////////////////////////////

typedef struct {
  ccc_ind_hdr_t hdr;
  ccc_ind_msg_t msg;
} ccc_ind_data_t;

typedef struct {
  ccc_ctrl_hdr_t hdr;
  ccc_ctrl_msg_t msg;
} ccc_ctrl_req_data_t;

//////////////////////////////////////
// E2 Setup Data
/////////////////////////////////////

typedef struct {
  ccc_func_def_t func_def;
} ccc_e2_setup_data_t;

//////////////////////////////////////
// RIC Service Update Data
/////////////////////////////////////

typedef struct {
  ccc_func_def_t func_def;
} ccc_ric_service_update_t;

//////////////////////////////////////
// Subscription Data
/////////////////////////////////////

typedef struct {
  ccc_event_trigger_t trigger;
  ccc_action_def_t action_def;
} ccc_sub_data_t;

void free_ccc_ind_data(ccc_ind_data_t* src);
ccc_ind_data_t cp_ccc_ind_data(const ccc_ind_data_t* src);
bool eq_ccc_ind_data(const ccc_ind_data_t* m0, const ccc_ind_data_t* m1);

void free_ccc_ctrl_req_data(ccc_ctrl_req_data_t* src);
ccc_ctrl_req_data_t cp_ccc_ctrl_req_data(const ccc_ctrl_req_data_t* src);
bool eq_ccc_ctrl_req_data(const ccc_ctrl_req_data_t* m0, const ccc_ctrl_req_data_t* m1);

void free_ccc_e2_setup_data(ccc_e2_setup_data_t* src);
void free_ccc_ric_service_update(ccc_ric_service_update_t* src);
void free_ccc_sub_data(ccc_sub_data_t* src);

#ifdef __cplusplus
}
#endif

#endif
