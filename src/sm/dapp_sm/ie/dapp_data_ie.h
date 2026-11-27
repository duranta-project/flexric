/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef DAPP_DATA_INFORMATION_ELEMENTS_H
#define DAPP_DATA_INFORMATION_ELEMENTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

//////////////////////////////////////
// DAPP inner E3 service model types
//////////////////////////////////////

#include "../e3/service_models/spectrum_sm/ir/spectrum_report.h"

#include "../e3/service_models/spectrum_sm/ir/spectrum_control.h"

/////////////////////////////////////
// dApp Subscription Info
/////////////////////////////////////

#include "ir/dapp_e3_subscription_list.h"

//////////////////////////////////////
// RIC Event Trigger Definition
/////////////////////////////////////

#include "ir/e2sm_dapp_ev_trg_frmt_1.h"

/////////////////////////////////////
// RIC Action Definition
/////////////////////////////////////

#include "ir/e2sm_dapp_action_def.h"

/////////////////////////////////////
// RIC Indication Header
/////////////////////////////////////

#include "ir/e2sm_dapp_ind_hdr_frmt_1.h"
#include "ir/e2sm_dapp_ind_hdr_frmt_2.h"

/////////////////////////////////////
// RIC Indication Message
/////////////////////////////////////

#include "ir/e2sm_dapp_ind_msg_frmt_1.h"
#include "ir/e2sm_dapp_ind_msg_frmt_2.h"

/////////////////////////////////////
// RIC Control Header
/////////////////////////////////////

#include "ir/e2sm_dapp_ctrl_hdr_frmt_1.h"

/////////////////////////////////////
// RIC Control Message
/////////////////////////////////////

#include "ir/e2sm_dapp_ctrl_msg_frmt_1.h"

/////////////////////////////////////
// RIC Control Outcome
/////////////////////////////////////

#include "ir/e2sm_dapp_ctrl_out_frmt_1.h"

/////////////////////////////////////
// RAN Function Definition
/////////////////////////////////////

#include "../../../lib/sm/ie/ran_function_name.h"
#include "ir/ran_func_def_ev_trig.h"
#include "ir/ran_func_def_report.h"
#include "ir/ran_func_def_ctrl.h"

//////////////////////////////////////
// DAPP inner E3 service model types
//////////////////////////////////////

typedef enum { DAPP_E3_SM_NONE = 0, DAPP_E3_SM_SPECTRUM = 1 } dapp_e3_sm_type_e;

typedef struct {
  dapp_e3_sm_type_e type;

  union {
    spectrum_sm_report_t spectrum; // for DAPP_E3_SM_SPECTRUM
  } u;
} dapp_e3_ind_payload_t;

__attribute__((visibility("default"))) void free_dapp_e3_ind_payload(dapp_e3_ind_payload_t* src);

__attribute__((visibility("default"))) dapp_e3_ind_payload_t cp_dapp_e3_ind_payload(dapp_e3_ind_payload_t const* src);

__attribute__((visibility("default"))) bool eq_dapp_e3_ind_payload(dapp_e3_ind_payload_t const* m0,
                                                                   dapp_e3_ind_payload_t const* m1);

typedef struct {
  dapp_e3_sm_type_e type;

  union {
    spectrum_sm_control_t spectrum; // for DAPP_E3_SM_SPECTRUM
  } u;

} dapp_e3_ctrl_payload_t;

__attribute__((visibility("default"))) void free_dapp_e3_ctrl_payload(dapp_e3_ctrl_payload_t* src);

__attribute__((visibility("default"))) dapp_e3_ctrl_payload_t cp_dapp_e3_ctrl_payload(dapp_e3_ctrl_payload_t const* src);

__attribute__((visibility("default"))) bool eq_dapp_e3_ctrl_payload(dapp_e3_ctrl_payload_t const* m0,
                                                                    dapp_e3_ctrl_payload_t const* m1);

//////////////////////////////////////
// RIC Event Trigger Definition
/////////////////////////////////////

typedef enum {
  FORMAT_1_E2SM_DAPP_EV_TRIGGER_FORMAT,
  END_E2SM_DAPP_EV_TRIGGER_FORMAT

} e2sm_dapp_ev_trigger_format_e;

typedef struct {
  e2sm_dapp_ev_trigger_format_e format;

  union {
    e2sm_dapp_ev_trg_frmt_1_t frmt_1;
  };
} e2sm_dapp_event_trigger_t;

void free_e2sm_dapp_event_trigger(e2sm_dapp_event_trigger_t* src);

e2sm_dapp_event_trigger_t cp_e2sm_dapp_event_trigger(e2sm_dapp_event_trigger_t const* src);

bool eq_e2sm_dapp_event_trigger(e2sm_dapp_event_trigger_t const* m0, e2sm_dapp_event_trigger_t const* m1);

//////////////////////////////////////
// RIC Action Definition
/////////////////////////////////////

typedef enum {
  FORMAT_1_E2SM_DAPP_ACTION_DEF,
  END_E2SM_DAPP_ACTION_DEF

} e2sm_dapp_action_def_format_e;

typedef struct {
  e2sm_dapp_action_def_format_e format;
  uint32_t ric_style_type;

  union {
    e2sm_dapp_action_def_frmt_1_t frmt_1;
  };
} e2sm_dapp_action_def_t;

void free_e2sm_dapp_action_def(e2sm_dapp_action_def_t* src);

e2sm_dapp_action_def_t cp_e2sm_dapp_action_def(e2sm_dapp_action_def_t const* src);

bool eq_e2sm_dapp_action_def(e2sm_dapp_action_def_t const* m0, e2sm_dapp_action_def_t const* m1);

//////////////////////////////////////
// RIC Indication Header
/////////////////////////////////////

typedef enum {
  FORMAT_1_E2SM_DAPP_IND_HDR,
  FORMAT_2_E2SM_DAPP_IND_HDR,
  END_E2SM_DAPP_IND_HDR

} e2sm_dapp_ind_hdr_format_e;

typedef struct {
  e2sm_dapp_ind_hdr_format_e format;

  union {
    e2sm_dapp_ind_hdr_frmt_1_t frmt_1;
    e2sm_dapp_ind_hdr_frmt_2_t frmt_2;
  };
} e2sm_dapp_ind_hdr_t;

__attribute__((visibility("default"))) void free_e2sm_dapp_ind_hdr(e2sm_dapp_ind_hdr_t* src);

e2sm_dapp_ind_hdr_t cp_e2sm_dapp_ind_hdr(e2sm_dapp_ind_hdr_t const* src);

bool eq_e2sm_dapp_ind_hdr(e2sm_dapp_ind_hdr_t const* m0, e2sm_dapp_ind_hdr_t const* m1);

//////////////////////////////////////
// RIC Indication Message
/////////////////////////////////////

typedef enum {
  FORMAT_1_E2SM_DAPP_IND_MSG,
  FORMAT_2_E2SM_DAPP_IND_MSG,
  END_E2SM_DAPP_IND_MSG

} e2sm_dapp_ind_msg_format_e;

typedef struct {
  e2sm_dapp_ind_msg_format_e format;

  union {
    e2sm_dapp_ind_msg_frmt_1_t frmt_1;
    e2sm_dapp_ind_msg_frmt_2_t frmt_2;
  };

} e2sm_dapp_ind_msg_t;

__attribute__((visibility("default"))) void free_e2sm_dapp_ind_msg(e2sm_dapp_ind_msg_t* src);

e2sm_dapp_ind_msg_t cp_e2sm_dapp_ind_msg(e2sm_dapp_ind_msg_t const* src);

bool eq_e2sm_dapp_ind_msg(e2sm_dapp_ind_msg_t const* m0, e2sm_dapp_ind_msg_t const* m1);

//////////////////////////////////////
// RIC Control Header
/////////////////////////////////////

typedef enum {
  FORMAT_1_E2SM_DAPP_CTRL_HDR,
  END_E2SM_DAPP_CTRL_HDR

} e2sm_dapp_ctrl_hdr_e;

typedef struct {
  e2sm_dapp_ctrl_hdr_e format;

  union {
    e2sm_dapp_ctrl_hdr_frmt_1_t frmt_1;
  };
} e2sm_dapp_ctrl_hdr_t;

void free_e2sm_dapp_ctrl_hdr(e2sm_dapp_ctrl_hdr_t* src);

e2sm_dapp_ctrl_hdr_t cp_e2sm_dapp_ctrl_hdr(e2sm_dapp_ctrl_hdr_t const* src);

bool eq_e2sm_dapp_ctrl_hdr(e2sm_dapp_ctrl_hdr_t const* m0, e2sm_dapp_ctrl_hdr_t const* m1);

//////////////////////////////////////
// RIC Control Message
/////////////////////////////////////

typedef enum {
  FORMAT_1_E2SM_DAPP_CTRL_MSG,
  END_E2SM_DAPP_CTRL_MSG

} e2sm_dapp_ctrl_msg_e;

typedef struct {
  e2sm_dapp_ctrl_msg_e format;

  union {
    e2sm_dapp_ctrl_msg_frmt_1_t frmt_1;
  };
} e2sm_dapp_ctrl_msg_t;

void free_e2sm_dapp_ctrl_msg(e2sm_dapp_ctrl_msg_t* src);

e2sm_dapp_ctrl_msg_t cp_e2sm_dapp_ctrl_msg(e2sm_dapp_ctrl_msg_t const* src);

bool eq_e2sm_dapp_ctrl_msg(e2sm_dapp_ctrl_msg_t const* m0, e2sm_dapp_ctrl_msg_t const* m1);

//////////////////////////////////////
// RIC Control Outcome
/////////////////////////////////////

typedef enum {
  FORMAT_1_E2SM_DAPP_CTRL_OUT,
  END_E2SM_DAPP_CTRL_OUT

} e2sm_dapp_ctrl_out_e;

typedef struct {
  e2sm_dapp_ctrl_out_e format;

  union {
    e2sm_dapp_ctrl_out_frmt_1_t frmt_1;
  };
} e2sm_dapp_ctrl_out_t;

void free_e2sm_dapp_ctrl_out(e2sm_dapp_ctrl_out_t* src);

e2sm_dapp_ctrl_out_t cp_e2sm_dapp_ctrl_out(e2sm_dapp_ctrl_out_t const* src);

bool eq_e2sm_dapp_ctrl_out(e2sm_dapp_ctrl_out_t const* m0, e2sm_dapp_ctrl_out_t const* m1);

//////////////////////////////////////
// RAN Function Definition
/////////////////////////////////////

typedef enum {
  DAPP_RIC_STYLE_E3_DATA_REPORT = 1,
  DAPP_RIC_STYLE_E3_SUBSCRIPTION_MAP = 2,
} dapp_ric_style_type_e;

typedef struct {
  //  RAN Function Name
  ran_function_name_t name;

  // RAN Function Definition for EVENT TRIGGER
  ran_func_def_ev_trig_dapp_sm_t* ev_trig;

  // RAN Function Definition for REPORT
  ran_func_def_report_dapp_sm_t* report;

  // RAN Function Definition for CONTROL
  ran_func_def_ctrl_dapp_sm_t* ctrl;

} e2sm_dapp_func_def_t;

__attribute__((visibility("default"))) void free_e2sm_dapp_func_def(e2sm_dapp_func_def_t* src);

e2sm_dapp_func_def_t cp_e2sm_dapp_func_def(e2sm_dapp_func_def_t const* src);

bool eq_e2sm_dapp_func_def(e2sm_dapp_func_def_t const* m0, e2sm_dapp_func_def_t const* m1);

/////////////////////////////////////////////////

///////////////
/// RIC Subscription
///////////////

typedef struct {
  e2sm_dapp_event_trigger_t et;
  e2sm_dapp_action_def_t* action_def;
} dapp_sub_data_t;

void free_dapp_sub_data(dapp_sub_data_t* ind);

bool eq_dapp_sub_data(dapp_sub_data_t const* m0, dapp_sub_data_t const* m1);

dapp_sub_data_t cp_dapp_sub_data(dapp_sub_data_t const* src);

///////////////
// RIC Indication
///////////////

typedef struct {
  e2sm_dapp_ind_hdr_t hdr;
  e2sm_dapp_ind_msg_t msg;
  dapp_e3_ind_payload_t e3;
} dapp_ind_data_t;

void free_dapp_ind_data(dapp_ind_data_t* ind);

bool eq_dapp_ind_data(dapp_ind_data_t const* m0, dapp_ind_data_t const* m1);

dapp_ind_data_t cp_dapp_ind_data(dapp_ind_data_t const* src);

///////////////
// RIC Control
///////////////

typedef struct {
  e2sm_dapp_ctrl_hdr_t hdr;
  e2sm_dapp_ctrl_msg_t msg;
  dapp_e3_ctrl_payload_t e3;
} dapp_ctrl_req_data_t;

void free_dapp_ctrl_req_data(dapp_ctrl_req_data_t* src);

bool eq_dapp_ctrl_req_data(dapp_ctrl_req_data_t const* m0, dapp_ctrl_req_data_t const* m1);

dapp_ctrl_req_data_t cp_dapp_ctrl_req_data(dapp_ctrl_req_data_t const* src);

typedef struct {
  e2sm_dapp_ctrl_out_t* out;
} dapp_ctrl_out_data_t;

///////////////
// E2 Setup
///////////////

typedef struct {
  e2sm_dapp_func_def_t ran_func_def;
} dapp_e2_setup_t;

///////////////
// RIC Service Update
///////////////

typedef struct {
  e2sm_dapp_func_def_t func_def;
} dapp_ric_service_update_t;

#ifdef __cplusplus
}
#endif

#endif
