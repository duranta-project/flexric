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


#ifndef SRS_DATA_INFORMATION_ELEMENTS_H
#define SRS_DATA_INFORMATION_ELEMENTS_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 9 Information Elements (IE) , RIC Event Trigger Definition, RIC Action Definition, RIC Indication Header, RIC Indication Message, RIC Call Process ID, RIC Control Header, RIC Control Message, RIC Control Outcome and RAN Function Definition defined by ORAN-WG3.E2SM-v01.00.00 at Section 5
 */


#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

//////////////////////////////////////
// RIC Event Trigger Definition
/////////////////////////////////////

typedef struct {
  uint32_t ms;
} srs_event_trigger_t;

void free_srs_event_trigger(srs_event_trigger_t* src); 

srs_event_trigger_t cp_srs_event_trigger( srs_event_trigger_t const* src);

bool eq_srs_event_trigger(srs_event_trigger_t const* m0, srs_event_trigger_t const* m1);



//////////////////////////////////////
// RIC Action Definition 
/////////////////////////////////////

// Maybe RAN Function ID like RC-SM? or leave it empty.
typedef struct {
  uint32_t dummy;  
} srs_action_def_t;

void free_srs_action_def(srs_action_def_t* src); 

srs_action_def_t cp_srs_action_def(srs_action_def_t* src);

bool eq_srs_action_def(srs_event_trigger_t* m0,  srs_event_trigger_t* m1);



//////////////////////////////////////
// RIC Indication Header 
/////////////////////////////////////


typedef struct{
  uint32_t dummy;  
} srs_ind_hdr_t;

void free_srs_ind_hdr(srs_ind_hdr_t* src); 

srs_ind_hdr_t cp_srs_ind_hdr(srs_ind_hdr_t const* src);

bool eq_srs_ind_hdr(srs_ind_hdr_t* m0, srs_ind_hdr_t* m1);

//////////////////////////////////////
// RIC Indication Message 
/////////////////////////////////////

typedef struct
{
  // Some fields from nfapi_nr_srs_indication_pdu_t
  uint16_t rnti;
} srs_ue_stats_impl_t;

srs_ue_stats_impl_t cp_srs_ue_stats_impl(srs_ue_stats_impl_t const* src);

typedef struct {
  uint32_t len_ue_stats;
  srs_ue_stats_impl_t* ue_stats;
  int64_t tstamp;
} srs_ind_msg_t;

void free_srs_ind_msg(srs_ind_msg_t* src); 

srs_ind_msg_t cp_srs_ind_msg(srs_ind_msg_t const* src);

bool eq_srs_ind_msg(srs_ind_msg_t* m0, srs_ind_msg_t* m1);


//////////////////////////////////////
// RIC Call Process ID 
/////////////////////////////////////

typedef struct {
  uint32_t dummy;
} srs_call_proc_id_t;

void free_srs_call_proc_id( srs_call_proc_id_t* src); 

srs_call_proc_id_t cp_srs_call_proc_id( srs_call_proc_id_t* src);

bool eq_srs_call_proc_id(srs_call_proc_id_t* m0, srs_call_proc_id_t* m1);

//////////////////////////////////////
// RIC Control Header 
/////////////////////////////////////

typedef struct {
  uint32_t dummy;
} srs_ctrl_hdr_t;

void free_srs_ctrl_hdr( srs_ctrl_hdr_t* src); 

srs_ctrl_hdr_t cp_srs_ctrl_hdr(srs_ctrl_hdr_t* src);

bool eq_srs_ctrl_hdr(srs_ctrl_hdr_t* m0, srs_ctrl_hdr_t* m1);

//////////////////////////////////////
// RIC Control Message 
/////////////////////////////////////

typedef struct {
  uint32_t dummy;
} srs_ctrl_msg_t;

void free_srs_ctrl_msg( srs_ctrl_msg_t* src); 

srs_ctrl_msg_t cp_srs_ctrl_msg(srs_ctrl_msg_t* src);

bool eq_srs_ctrl_msg(srs_ctrl_msg_t* m0, srs_ctrl_msg_t* m1);


//////////////////////////////////////
// RIC Control Outcome 
/////////////////////////////////////

typedef enum{
  SRS_CTRL_OUT_OK,


  SRS_CTRL_OUT_END
} srs_ctrl_out_e;

typedef struct {
  uint32_t ans;  
} srs_ctrl_out_t;

void free_srs_ctrl_out(srs_ctrl_out_t* src); 

srs_ctrl_out_t cp_srs_ctrl_out(srs_ctrl_out_t* src);

bool eq_srs_ctrl_out(srs_ctrl_out_t* m0, srs_ctrl_out_t* m1);


//////////////////////////////////////
// RAN Function Definition 
/////////////////////////////////////

typedef struct {
  size_t len;
  uint8_t* buf;
} srs_func_def_t;

void free_srs_func_def(srs_func_def_t* src); 

srs_func_def_t cp_srs_func_def(srs_func_def_t const* src);

bool eq_srs_func_def(srs_func_def_t const* m0, srs_func_def_t const* m1);


/////////////////////////////////////////////////
//////////////////////////////////////////////////
/////////////////////////////////////////////////


/*
 * O-RAN defined 5 Procedures: RIC Subscription, RIC Indication, RIC Control, E2 Setup and RIC Service Update 
 * */


///////////////
/// RIC Subscription
///////////////

typedef struct{
  srs_event_trigger_t et; 
  srs_action_def_t* ad;
} srs_sub_data_t;

///////////////
// RIC Indication
///////////////

typedef struct{
  srs_ind_hdr_t hdr;
  srs_ind_msg_t msg;
  srs_call_proc_id_t* proc_id;
} srs_ind_data_t;

srs_ind_data_t cp_srs_ind_data( srs_ind_data_t const* src);

void free_srs_ind_data(srs_ind_data_t* ind);

///////////////
// RIC Control
///////////////

typedef struct{
  srs_ctrl_hdr_t hdr;
  srs_ctrl_msg_t msg;
} srs_ctrl_req_data_t;

typedef struct{
  srs_ctrl_out_t* out;
} srs_ctrl_out_data_t;

///////////////
// E2 Setup
///////////////

typedef struct{
  srs_func_def_t func_def;
} srs_e2_setup_data_t;

///////////////
// RIC Service Update
///////////////

typedef struct{
  srs_func_def_t func_def;
} srs_ric_service_update_t;

#ifdef __cplusplus
}
#endif




#endif

