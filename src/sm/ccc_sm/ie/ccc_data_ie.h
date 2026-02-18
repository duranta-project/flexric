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
// CCC Service Model Style Types
/////////////////////////////////////
// 
// Usage Examples:
//
//   1. Control Header with Style Type:
//      ccc_ctrl_hdr_t hdr = {0};
//      hdr.format = FORMAT_1_E2SM_CCC_CTRL_HDR;
//      hdr.format1.ric_style_type = CCC_CTRL_SERVICE_STYLE_TYPE_1;  // Node-level control
//
//   2. RAN Configuration Structure with String Name:
//      ccc_ran_configuration_structure_t config = {0};
//      config.ran_configuration_structure_name = strdup(CCC_RAN_STRUCT_NAME_O_RU_INFO);
//      // or cell-level:
//      config.ran_configuration_structure_name = strdup(CCC_RAN_STRUCT_NAME_O_NR_CELL_DU);
//
//   3. Custom/Vendor-specific Structure:
//      config.ran_configuration_structure_name = strdup("MyVendor-CustomConfig");
//

// Event Trigger Style Types (Section 7.5 of E2SM-CCC spec)
typedef enum {
  CCC_EV_TRIGGER_STYLE_TYPE_1 = 1,  // Node-level Configuration Change
  CCC_EV_TRIGGER_STYLE_TYPE_2 = 2,  // Cell-level Configuration Change
  CCC_EV_TRIGGER_STYLE_TYPE_3 = 3,  // Periodic Reporting
  
  END_CCC_EV_TRIGGER_STYLE_TYPE
} ccc_event_trigger_style_type_e;

// Report Service Style Types (Section 7.6)
typedef enum {
  CCC_REPORT_STYLE_TYPE_1 = 1,  // Node-level Configuration Report
  CCC_REPORT_STYLE_TYPE_2 = 2,  // Cell-level Configuration Report
  
  END_CCC_REPORT_STYLE_TYPE
} ccc_report_style_type_e;

// Control Service Style Types (Section 7.7)
typedef enum {
  CCC_CTRL_SERVICE_STYLE_TYPE_1 = 1,  // Node-level RAN Configuration Control
  CCC_CTRL_SERVICE_STYLE_TYPE_2 = 2,  // Cell-level RAN Configuration Control
  
  END_CCC_CTRL_SERVICE_STYLE_TYPE
} ccc_control_service_style_type_e;

// Query Service Style Types (Section 7.8)
typedef enum {
  CCC_QUERY_SERVICE_STYLE_TYPE_1 = 1,  // Node-level Configuration Query
  CCC_QUERY_SERVICE_STYLE_TYPE_2 = 2,  // Cell-level Configuration Query
  
  END_CCC_QUERY_SERVICE_STYLE_TYPE
} ccc_query_service_style_type_e;

// RAN Configuration Structure Names (Section 8.2)
// These are strings in the spec, extensible and vendor-specific
// 
// Common Standard Structure Names:
#define CCC_RAN_STRUCT_NAME_O_RU_INFO                  "O-RU-Info"
#define CCC_RAN_STRUCT_NAME_O_DU_INFO                  "O-DU-Info"
#define CCC_RAN_STRUCT_NAME_O_CU_CP_INFO               "O-CU-CP-Info"
#define CCC_RAN_STRUCT_NAME_O_CU_UP_INFO               "O-CU-UP-Info"
#define CCC_RAN_STRUCT_NAME_CELL_INFO                  "Cell-Info"
#define CCC_RAN_STRUCT_NAME_BWP_CONFIG                 "BWP-Config"
#define CCC_RAN_STRUCT_NAME_CELL_DTXDRX_CONFIG         "Cell-DTX-DRX-Config"
#define CCC_RAN_STRUCT_NAME_CES_MANAGEMENT_FUNCTION    "CES-Management-Function"
// Cell-level structures (examples from spec)
#define CCC_RAN_STRUCT_NAME_O_NES_POLICY               "O-NESPolicy"
#define CCC_RAN_STRUCT_NAME_O_NR_CELL_DU               "O-NRCellDU"
#define CCC_RAN_STRUCT_NAME_O_NR_CELL_CU               "O-NRCellCU"

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

//////////////////////////////////////
// Cell Identifiers
/////////////////////////////////////

// EUTRA CGI
typedef struct {
  uint8_t plmn_identity[3];  // MCC + MNC
  uint32_t e_utra_cell_identity;  // 28 bits
} ccc_eutra_cgi_t;

// NR CGI
typedef struct {
  uint8_t plmn_identity[3];
  uint64_t nr_cell_identity;  // 36 bits
} ccc_nr_cgi_t;

typedef enum {
  CCC_CGI_EUTRA = 0,
  CCC_CGI_NR = 1
} ccc_cgi_type_e;

typedef struct {
  ccc_cgi_type_e type;
  union {
    ccc_eutra_cgi_t eutra_cgi;
    ccc_nr_cgi_t nr_cgi;
  };
} ccc_cgi_t;

//////////////////////////////////////
// Energy Saving Structures
/////////////////////////////////////

// Energy Saving Capability Common Info
typedef struct {
  bool st8_ready_message_supported;
  bool sleep_duration_extension_supported;
  bool emergency_wake_up_command_supported;
} ccc_energy_saving_capability_common_info_t;

// Energy Saving State
typedef enum {
  CCC_ES_STATE_NOT_SUPPORTED = 0,
  CCC_ES_STATE_AWAKE = 1,
  CCC_ES_STATE_SLEEP = 2,
  CCC_ES_STATE_DEEP_SLEEP = 3
} ccc_energy_saving_state_e;

// Energy Saving Control
typedef enum {
  CCC_ES_CONTROL_DISABLED = 0,
  CCC_ES_CONTROL_ENABLED = 1
} ccc_energy_saving_control_e;

// CES Management Function
typedef struct {
  uint32_t ces_switch;
  ccc_energy_saving_state_e energy_saving_state;
  ccc_energy_saving_control_e energy_saving_control;
} ccc_ces_management_function_t;

//////////////////////////////////////
// RAN Configuration Structures
/////////////////////////////////////

// RAN Configuration Structure Name (common names from spec)
typedef enum {
  CCC_RAN_CONFIG_O_RU_INFO = 0,
  CCC_RAN_CONFIG_O_DU_INFO = 1,
  CCC_RAN_CONFIG_O_CU_CP_INFO = 2,
  CCC_RAN_CONFIG_O_CU_UP_INFO = 3,
  CCC_RAN_CONFIG_O_CELL_INFO = 4,
  CCC_RAN_CONFIG_O_BWP_INFO = 5,
  CCC_RAN_CONFIG_O_NES_POLICY = 6,
  CCC_RAN_CONFIG_O_CES_MGMT_FUNCTION = 7,
  CCC_RAN_CONFIG_O_CELL_DTXDRX_CONFIG = 8,
  CCC_RAN_CONFIG_CUSTOM = 999
} ccc_ran_config_name_e;

// Generic RAN Configuration Structure
typedef struct {
  ccc_ran_config_name_e name_enum;
  char* ran_configuration_structure_name;  // String representation
  char* values_of_attributes;  // JSON or serialized data
  size_t values_len;
} ccc_ran_configuration_structure_t;

// RAN Configuration Structure Item (for lists)
typedef struct {
  ccc_ran_configuration_structure_t* items;
  size_t len;
} ccc_ran_configuration_structure_list_t;

// Configuration Structure Accepted (for control outcome)
typedef struct {
  char* ran_configuration_structure_name;
  uint64_t applied_timestamp;
} ccc_config_structure_accepted_t;

// Configuration Structure Failed (for control outcome)
typedef struct {
  char* ran_configuration_structure_name;
  char* cause;  // Failure reason
} ccc_config_structure_failed_t;

//////////////////////////////////////
// Cell and BWP Configuration
/////////////////////////////////////

// Subcarrier Spacing
typedef enum {
  CCC_SCS_15KHZ = 0,
  CCC_SCS_30KHZ = 1,
  CCC_SCS_60KHZ = 2,
  CCC_SCS_120KHZ = 3,
  CCC_SCS_240KHZ = 4
} ccc_subcarrier_spacing_e;

// Cyclic Prefix
typedef enum {
  CCC_CP_NORMAL = 0,
  CCC_CP_EXTENDED = 1
} ccc_cyclic_prefix_e;

// BWP Context
typedef enum {
  CCC_BWP_DOWNLINK = 0,
  CCC_BWP_UPLINK = 1
} ccc_bwp_context_e;

// BWP Configuration
typedef struct {
  ccc_bwp_context_e bwp_context;
  bool is_initial_bwp;
  ccc_subcarrier_spacing_e sub_carrier_spacing;
  ccc_cyclic_prefix_e cyclic_prefix;
  uint32_t start_rb;
  uint32_t number_of_rbs;
} ccc_bwp_config_t;

// DRX/DTX Configuration Type
typedef enum {
  CCC_DRX_CONFIG = 0,
  CCC_DTX_CONFIG = 1
} ccc_dtxdrx_config_type_e;

// DRX/DTX Activation Status
typedef enum {
  CCC_DTXDRX_INACTIVE = 0,
  CCC_DTXDRX_ACTIVE = 1
} ccc_dtxdrx_activation_status_e;

// Cell DTX/DRX Configuration
typedef struct {
  uint32_t on_duration_timer;
  uint32_t cycle_start_offset;
  uint32_t slot_offset;
  ccc_dtxdrx_config_type_e config_type;
  ccc_dtxdrx_activation_status_e activation_status;
  bool l1_activation;
} ccc_cell_dtxdrx_config_t;

//////////////////////////////////////
// Cell Information
/////////////////////////////////////

// Administrative State
typedef enum {
  CCC_ADMIN_STATE_LOCKED = 0,
  CCC_ADMIN_STATE_UNLOCKED = 1,
  CCC_ADMIN_STATE_SHUTTINGDOWN = 2
} ccc_administrative_state_e;

// Cell Information Structure
typedef struct {
  ccc_cgi_t cgi;
  uint32_t cell_local_id;
  ccc_administrative_state_e admin_state;
  char* additional_info;  // JSON or other format
  size_t additional_info_len;
} ccc_cell_info_t;

// List of Cells
typedef struct {
  ccc_cell_info_t* cells;
  size_t len;
} ccc_cell_list_t;

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

typedef enum {
  FORMAT_1_E2SM_CCC_ACTION_DEF = 1,
  FORMAT_2_E2SM_CCC_ACTION_DEF = 2,
  END_E2SM_CCC_ACTION_DEF
} e2sm_ccc_action_def_format_e;

// Action Definition Format 1 - Node Level RAN Configuration
typedef struct {
  ccc_items_element_t list_of_node_level_ran_configuration_structures_for_adf;
} ccc_action_def_format1_t;

// Action Definition Format 2 - Cell Configurations to be Reported
typedef struct {
  ccc_items_element_t list_of_cell_configurations_to_be_reported_for_adf;
} ccc_action_def_format2_t;

typedef struct {
  e2sm_ccc_action_def_format_e format;
  union {
    ccc_action_def_format1_t format1;
    ccc_action_def_format2_t format2;
    uint32_t action_id;  // Simplified (backward compatible)
  };
} ccc_action_def_t;

void free_ccc_action_def(ccc_action_def_t* src);
ccc_action_def_t cp_ccc_action_def(const ccc_action_def_t* src);
bool eq_ccc_action_def(const ccc_action_def_t* m0, const ccc_action_def_t* m1);

//////////////////////////////////////
// RIC Indication Header
/////////////////////////////////////

typedef enum {
  FORMAT_1_E2SM_CCC_IND_HDR = 1,
  END_E2SM_CCC_IND_HDR
} e2sm_ccc_ind_hdr_format_e;

// Indication Header Format 1
typedef struct {
  char* indication_reason;  // Change type
  uint64_t event_time;
} ccc_ind_hdr_format1_t;

typedef struct {
  e2sm_ccc_ind_hdr_format_e format;
  union {
    ccc_ind_hdr_format1_t format1;
    uint32_t timestamp;  // Simplified format (backward compatible)
  };
} ccc_ind_hdr_t;

void free_ccc_ind_hdr(ccc_ind_hdr_t* src);
ccc_ind_hdr_t cp_ccc_ind_hdr(const ccc_ind_hdr_t* src);
bool eq_ccc_ind_hdr(const ccc_ind_hdr_t* m0, const ccc_ind_hdr_t* m1);

//////////////////////////////////////
// RIC Indication Message
/////////////////////////////////////

typedef enum {
  FORMAT_1_E2SM_CCC_IND_MSG = 1,
  FORMAT_2_E2SM_CCC_IND_MSG = 2,
  END_E2SM_CCC_IND_MSG
} e2sm_ccc_ind_msg_format_e;

// Indication Message Format 1 - Configuration Structures Reported
typedef struct {
  ccc_items_element_t list_of_configuration_structures_reported;
} ccc_ind_msg_format1_t;

// Indication Message Format 2 - Cells Reported
typedef struct {
  ccc_items_element_t list_of_cells_reported;
} ccc_ind_msg_format2_t;

typedef struct {
  e2sm_ccc_ind_msg_format_e format;
  union {
    ccc_ind_msg_format1_t format1;
    ccc_ind_msg_format2_t format2;
    // Simplified format (backward compatible)
    struct {
      char* json_payload;
      size_t payload_len;
    };
  };
} ccc_ind_msg_t;

void free_ccc_ind_msg(ccc_ind_msg_t* src);
ccc_ind_msg_t cp_ccc_ind_msg(const ccc_ind_msg_t* src);
bool eq_ccc_ind_msg(const ccc_ind_msg_t* m0, const ccc_ind_msg_t* m1);

//////////////////////////////////////
// RIC Control Header
/////////////////////////////////////

typedef enum {
  FORMAT_1_E2SM_CCC_CTRL_HDR = 1,
  END_E2SM_CCC_CTRL_HDR
} e2sm_ccc_ctrl_hdr_format_e;

// Control Header Format 1
typedef struct {
  ccc_control_service_style_type_e ric_style_type;  // Style 1 or 2
} ccc_ctrl_hdr_format1_t;

typedef struct {
  e2sm_ccc_ctrl_hdr_format_e format;
  union {
    ccc_ctrl_hdr_format1_t format1;
    uint32_t control_type;  // Simplified (backward compatible)
  };
} ccc_ctrl_hdr_t;

void free_ccc_ctrl_hdr(ccc_ctrl_hdr_t* src);
ccc_ctrl_hdr_t cp_ccc_ctrl_hdr(const ccc_ctrl_hdr_t* src);
bool eq_ccc_ctrl_hdr(const ccc_ctrl_hdr_t* m0, const ccc_ctrl_hdr_t* m1);

//////////////////////////////////////
// RIC Control Message
/////////////////////////////////////

typedef enum {
  FORMAT_1_E2SM_CCC_CTRL_MSG = 1,
  FORMAT_2_E2SM_CCC_CTRL_MSG = 2,
  END_E2SM_CCC_CTRL_MSG
} e2sm_ccc_ctrl_msg_format_e;

// Control Message Format 1 - Configuration Structures
typedef struct {
  ccc_items_element_t list_of_configuration_structures;
} ccc_ctrl_msg_format1_t;

// Control Message Format 2 - Cells Controlled
typedef struct {
  ccc_items_element_t list_of_cells_controlled;
} ccc_ctrl_msg_format2_t;

typedef struct {
  e2sm_ccc_ctrl_msg_format_e format;
  union {
    ccc_ctrl_msg_format1_t format1;
    ccc_ctrl_msg_format2_t format2;
    // Simplified (backward compatible)
    struct {
      char* json_payload;
      size_t payload_len;
    };
  };
} ccc_ctrl_msg_t;

void free_ccc_ctrl_msg(ccc_ctrl_msg_t* src);
ccc_ctrl_msg_t cp_ccc_ctrl_msg(const ccc_ctrl_msg_t* src);
bool eq_ccc_ctrl_msg(const ccc_ctrl_msg_t* m0, const ccc_ctrl_msg_t* m1);

//////////////////////////////////////
// RIC Control Outcome
/////////////////////////////////////

typedef enum {
  FORMAT_1_E2SM_CCC_CTRL_OUT = 1,
  FORMAT_2_E2SM_CCC_CTRL_OUT = 2,
  END_E2SM_CCC_CTRL_OUT
} e2sm_ccc_ctrl_out_format_e;

// Control Outcome Format 1
typedef struct {
  uint64_t received_timestamp;
  ccc_items_element_t ran_configuration_structures_accepted_list;
  ccc_items_element_t ran_configuration_structures_failed_list;
} ccc_ctrl_out_format1_t;

// Control Outcome Format 2
typedef struct {
  uint64_t received_timestamp;
  ccc_items_element_t list_of_cells_for_control_outcome;
} ccc_ctrl_out_format2_t;

typedef struct {
  e2sm_ccc_ctrl_out_format_e format;
  union {
    ccc_ctrl_out_format1_t format1;
    ccc_ctrl_out_format2_t format2;
    uint32_t outcome;  // Simplified (backward compatible)
  };
} ccc_ctrl_out_t;

void free_ccc_ctrl_out(ccc_ctrl_out_t* src);
ccc_ctrl_out_t cp_ccc_ctrl_out(const ccc_ctrl_out_t* src);
bool eq_ccc_ctrl_out(const ccc_ctrl_out_t* m0, const ccc_ctrl_out_t* m1);

//////////////////////////////////////
// RIC Query Definition
/////////////////////////////////////

typedef enum {
  FORMAT_1_E2SM_CCC_QUERY_DEF = 1,
  FORMAT_2_E2SM_CCC_QUERY_DEF = 2,
  END_E2SM_CCC_QUERY_DEF
} e2sm_ccc_query_def_format_e;

// Query Definition Format 1 - Node Level
typedef struct {
  ccc_items_element_t list_of_nodelevel_ran_configuration_structures_for_query;
} ccc_query_def_format1_t;

// Query Definition Format 2 - Cell Level
typedef struct {
  ccc_items_element_t list_of_cells_to_be_queried_for_query;
} ccc_query_def_format2_t;

typedef struct {
  e2sm_ccc_query_def_format_e format;
  union {
    ccc_query_def_format1_t format1;
    ccc_query_def_format2_t format2;
  };
} ccc_query_def_t;

void free_ccc_query_def(ccc_query_def_t* src);
ccc_query_def_t cp_ccc_query_def(const ccc_query_def_t* src);
bool eq_ccc_query_def(const ccc_query_def_t* m0, const ccc_query_def_t* m1);

//////////////////////////////////////
// RIC Query Outcome
/////////////////////////////////////

typedef enum {
  FORMAT_1_E2SM_CCC_QUERY_OUT = 1,
  FORMAT_2_E2SM_CCC_QUERY_OUT = 2,
  END_E2SM_CCC_QUERY_OUT
} e2sm_ccc_query_out_format_e;

// Query Outcome Format 1
typedef struct {
  ccc_items_element_t list_of_configuration_structures_to_be_reported_for_query;
} ccc_query_out_format1_t;

// Query Outcome Format 2
typedef struct {
  ccc_items_element_t list_of_cells_to_be_reported_for_query;
} ccc_query_out_format2_t;

typedef struct {
  e2sm_ccc_query_out_format_e format;
  union {
    ccc_query_out_format1_t format1;
    ccc_query_out_format2_t format2;
  };
} ccc_query_out_t;

void free_ccc_query_out(ccc_query_out_t* src);
ccc_query_out_t cp_ccc_query_out(const ccc_query_out_t* src);
bool eq_ccc_query_out(const ccc_query_out_t* m0, const ccc_query_out_t* m1);

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
