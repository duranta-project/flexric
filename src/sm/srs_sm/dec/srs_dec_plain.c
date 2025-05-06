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



#include "srs_dec_plain.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

srs_event_trigger_t srs_dec_event_trigger_plain(size_t len, uint8_t const ev_tr[len])
{
  srs_event_trigger_t ev = {0};
  memcpy(&ev.ev_trigger_cond_id, ev_tr, sizeof(ev.ev_trigger_cond_id));
  return ev;
}

srs_action_def_t srs_dec_action_def_plain(size_t len, uint8_t const action_def[len])
{
  assert(len == sizeof(srs_action_def_t));
  srs_action_def_t act_def;
  memcpy(&act_def, action_def, len);
  return act_def;
}

srs_ind_hdr_t srs_dec_ind_hdr_plain(size_t len, uint8_t const ind_hdr[len])
{
  assert(len == sizeof(srs_ind_hdr_t)); 
  srs_ind_hdr_t ret;
  memcpy(&ret, ind_hdr, len);
  return ret;
}

srs_ind_msg_t srs_dec_ind_msg_plain(size_t len, uint8_t const ind_msg[len])
{
//  assert(len == sizeof(srs_ind_msg_t)); 
  srs_ind_msg_t ret;

  static_assert(sizeof(uint32_t) == sizeof(ret.len), "Different sizes!");

  const size_t len_sizeof = sizeof(ret.len);
  memcpy(&ret.len, ind_msg, len_sizeof);

  if(ret.len > 0){
    ret.indication_stats = calloc(ret.len, sizeof(srs_indication_stats_impl_t));
    assert(ret.indication_stats != NULL && "Memory exhausted!");
  }
  
  void* ptr = (void*)&ind_msg[len_sizeof];
  for(uint32_t i = 0; i < ret.len; ++i){
    srs_indication_stats_impl_t* ret_stats = &ret.indication_stats[i];

    memcpy(&ret_stats->rnti, ptr, sizeof(ret_stats->rnti));
    ptr += sizeof(ret_stats->rnti);

    memcpy(&ret_stats->srs_unpacked_pdu.len, ptr, sizeof(ret_stats->srs_unpacked_pdu.len));
    ptr += sizeof(ret_stats->srs_unpacked_pdu.len);
    ret_stats->srs_unpacked_pdu.buf = malloc(ret_stats->srs_unpacked_pdu.len);
    memcpy(ret_stats->srs_unpacked_pdu.buf, ptr, ret_stats->srs_unpacked_pdu.len);
    ptr += ret_stats->srs_unpacked_pdu.len;
  }

  memcpy(&ret.tstamp, ptr, sizeof(ret.tstamp));
  ptr += sizeof(ret.tstamp);
  assert(ptr == ind_msg + len && "data layout mismatch");

  return ret;
}

srs_call_proc_id_t srs_dec_call_proc_id_plain(size_t len, uint8_t const call_proc_id[len])
{
  assert(0!=0 && "Not implemented");
  assert(call_proc_id != NULL);
}

srs_ctrl_hdr_t srs_dec_ctrl_hdr_plain(size_t len, uint8_t const ctrl_hdr[len])
{
  assert(len == sizeof(srs_ctrl_hdr_t)); 
  srs_ctrl_hdr_t ret;
  memcpy(&ret, ctrl_hdr, len);
  return ret;
}

srs_ctrl_msg_t srs_dec_ctrl_msg_plain(size_t len, uint8_t const ctrl_msg[len])
{
  assert(len == sizeof(srs_ctrl_msg_t)); 
  srs_ctrl_msg_t ret;
  memcpy(&ret, ctrl_msg, len);
  return ret;
}

srs_ctrl_out_t srs_dec_ctrl_out_plain(size_t len, uint8_t const ctrl_out[len]) 
{
  assert(0!=0 && "Not implemented");
  assert(ctrl_out != NULL);
}

srs_func_def_t srs_dec_func_def_plain(size_t len, uint8_t const func_def[len])
{
  assert(0!=0 && "Not implemented");
  assert(func_def != NULL);
}


