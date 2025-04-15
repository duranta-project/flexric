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


#include "srs_data_ie.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "../../../util/alg_ds/alg/eq_float.h"


//////////////////////////////////////
// RIC Event Trigger Definition
/////////////////////////////////////

void free_srs_event_trigger(srs_event_trigger_t* src)
{
  assert(src != NULL);
  assert(0!=0 && "Not implemented" ); 
}

srs_event_trigger_t cp_srs_event_trigger( srs_event_trigger_t const* src)
{
  assert(src != NULL);
  assert(0!=0 && "Not implemented" ); 

  srs_event_trigger_t et = {0};
  return et;
}

bool eq_srs_event_trigger(srs_event_trigger_t const* m0, srs_event_trigger_t const* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);

  assert(0!=0 && "Not implemented" ); 

  return true;
}


//////////////////////////////////////
// RIC Action Definition 
/////////////////////////////////////

void free_srs_action_def(srs_action_def_t* src)
{
  assert(src != NULL);

  assert(0!=0 && "Not implemented" ); 
}

srs_action_def_t cp_srs_action_def(srs_action_def_t* src)
{
  assert(src != NULL);

  assert(0!=0 && "Not implemented" ); 
  srs_action_def_t ad = {0};
  return ad;
}

bool eq_srs_action_def(srs_event_trigger_t* m0,  srs_event_trigger_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);

  assert(0!=0 && "Not implemented" ); 

  return true;
}


//////////////////////////////////////
// RIC Indication Header 
/////////////////////////////////////


void free_srs_ind_hdr(srs_ind_hdr_t* src)
{
  assert(src != NULL);
  (void)src;
}

srs_ind_hdr_t cp_srs_ind_hdr(srs_ind_hdr_t const* src)
{
  assert(src != NULL);
  srs_ind_hdr_t dst = {0}; 
  dst.dummy = src->dummy;
  return dst;
}

bool eq_srs_ind_hdr(srs_ind_hdr_t* m0, srs_ind_hdr_t* m1)
{
  assert(m0 != 0);
  assert(m1 != 0);

  if(m0->dummy != m1->dummy)
    return false;
  return true;
}


//////////////////////////////////////
// RIC Indication Message 
/////////////////////////////////////

void free_srs_ind_msg(srs_ind_msg_t* src)
{
  assert(src != NULL);
  if(src->len > 0){
    assert(src->indication_stats != NULL);
    free(src->indication_stats);
  }
}

srs_indication_stats_impl_t cp_srs_indication_stats_impl(srs_indication_stats_impl_t const* src)
{
  assert(src != NULL);

  srs_indication_stats_impl_t dst = { .rnti = src->rnti}; 

  return dst;
}

srs_ind_msg_t cp_srs_ind_msg( srs_ind_msg_t const* src)
{
  assert(src != NULL);

  srs_ind_msg_t dst = {0};

  dst.len = src->len;
  if(dst.len > 0){
    dst.indication_stats = calloc(dst.len, sizeof( srs_indication_stats_impl_t) );
    assert(dst.indication_stats != NULL && "Memory exhausted");
  }

  for(size_t i = 0; i < dst.len; ++i){
    dst.indication_stats[i] = cp_srs_indication_stats_impl(&src->indication_stats[i]); 
  }

  dst.tstamp = src->tstamp; 

  return dst;
}

bool eq_srs_ind_msg(srs_ind_msg_t* m0, srs_ind_msg_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);

  if(m0->len != m1->len || m0->tstamp != m1->tstamp)
    return false;

  for(uint32_t i = 0 ; i < m0->len; ++i){
    srs_indication_stats_impl_t* ue0 = &m0->indication_stats[i]; 
    srs_indication_stats_impl_t* ue1 = &m1->indication_stats[i]; 

    if(
        ue0->rnti != ue1->rnti
      )
      return false;
  }
  return true;
}

//////////////////////////////////////
// RIC Call Process ID 
/////////////////////////////////////

void free_srs_call_proc_id(srs_call_proc_id_t* src)
{
  // Note that the src could be NULL
  free(src);
}

srs_call_proc_id_t cp_srs_call_proc_id( srs_call_proc_id_t* src)
{
  assert(src != NULL); 
  srs_call_proc_id_t dst = {0};

  dst.dummy = src->dummy;

  return dst;
}

bool eq_srs_call_proc_id(srs_call_proc_id_t* m0, srs_call_proc_id_t* m1)
{
  if(m0 == NULL && m1 == NULL)
    return true;
  if(m0 == NULL)
    return false;
  if(m1 == NULL)
    return false;

  if(m0->dummy != m1->dummy)
    return false;

  return true;
}


//////////////////////////////////////
// RIC Control Header 
/////////////////////////////////////

void free_srs_ctrl_hdr( srs_ctrl_hdr_t* src)
{

  assert(src != NULL);
  assert(0!=0 && "Not implemented" ); 
}

srs_ctrl_hdr_t cp_srs_ctrl_hdr(srs_ctrl_hdr_t* src)
{
  assert(src != NULL);
  assert(0!=0 && "Not implemented" ); 
  srs_ctrl_hdr_t ret = {0};
  return ret;
}

bool eq_srs_ctrl_hdr(srs_ctrl_hdr_t* m0, srs_ctrl_hdr_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);

  assert(0!=0 && "Not implemented" ); 

  return true;
}


//////////////////////////////////////
// RIC Control Message 
/////////////////////////////////////


void free_srs_ctrl_msg( srs_ctrl_msg_t* src)
{
  assert(src != NULL);

  assert(0!=0 && "Not implemented" ); 
}

srs_ctrl_msg_t cp_srs_ctrl_msg(srs_ctrl_msg_t* src)
{
  assert(src != NULL);

  assert(0!=0 && "Not implemented" ); 
  srs_ctrl_msg_t ret = {0};
  return ret;
}

bool eq_srs_ctrl_msg(srs_ctrl_msg_t* m0, srs_ctrl_msg_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);

  assert(0!=0 && "Not implemented" ); 

  return true;
}


//////////////////////////////////////
// RIC Control Outcome 
/////////////////////////////////////

void free_srs_ctrl_out(srs_ctrl_out_t* src)
{
  assert(src != NULL);

  assert(0!=0 && "Not implemented" ); 
}

srs_ctrl_out_t cp_srs_ctrl_out(srs_ctrl_out_t* src)
{
  assert(src != NULL);

  assert(0!=0 && "Not implemented" ); 
  srs_ctrl_out_t ret = {0}; 
  return ret;
}

bool eq_srs_ctrl_out(srs_ctrl_out_t* m0, srs_ctrl_out_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);

  assert(0!=0 && "Not implemented" ); 

  return true;
}


//////////////////////////////////////
// RAN Function Definition 
/////////////////////////////////////

void free_srs_func_def(srs_func_def_t* src)
{
  assert(src != NULL);
  free(src->buf);
}

srs_func_def_t cp_srs_func_def(srs_func_def_t const* src)
{
  assert(src != NULL);

  srs_func_def_t dst = {.len = src->len};
  if(src->len > 0){
    dst.buf = calloc(dst.len, sizeof(uint8_t)); 
    assert(dst.buf != NULL && "memory exhausted");
    memcpy(dst.buf, src->buf, dst.len);
  }

  return dst;
}

bool eq_srs_func_def(srs_func_def_t const* m0, srs_func_def_t const* m1)
{
  if(m0 == m1)
    return true;

  if(m0 == NULL || m1 == NULL)
    return false;

  if(m0->len != m1->len)
    return false;

  int rc = memcmp(m0, m1, m0->len);
  return rc == 0;
}

///////////////
// RIC Indication
///////////////

srs_ind_data_t cp_srs_ind_data( srs_ind_data_t const* src)
{
  assert(src != NULL);
  srs_ind_data_t dst = {0};
  dst.hdr = cp_srs_ind_hdr(&src->hdr);
  dst.msg = cp_srs_ind_msg(&src->msg);
  
  if(src->proc_id != NULL){
    dst.proc_id = malloc(sizeof(srs_call_proc_id_t)); 
    assert(dst.proc_id != NULL && "Memory exhausted");
    *dst.proc_id = cp_srs_call_proc_id(src->proc_id);
  }

  return dst;
}

void free_srs_ind_data(srs_ind_data_t* ind)
{
  assert(ind != NULL);
  free_srs_ind_hdr(&ind->hdr);
  free_srs_ind_msg(&ind->msg);
  free_srs_call_proc_id(ind->proc_id);
}


