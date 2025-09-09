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



#include "srs_enc_plain.h"

#include <assert.h>
#include <stdlib.h>


byte_array_t srs_enc_event_trigger_plain(srs_event_trigger_t const* event_trigger)
{
  assert(event_trigger != NULL);
  byte_array_t  ba = {0};
 
  ba.len = sizeof(event_trigger->ev_trigger_cond_id);
  ba.buf = malloc(ba.len);
  assert(ba.buf != NULL && "Memory exhausted");

  memcpy(ba.buf, &event_trigger->ev_trigger_cond_id, ba.len);

  return ba;
}

byte_array_t srs_enc_action_def_plain(srs_action_def_t const* action_def)
{

  assert(action_def != NULL);
  byte_array_t  ba = {0};

  ba.len = sizeof(action_def->dummy);
  ba.buf = malloc(ba.len);
  assert(ba.buf != NULL && "Memory exhausted");

  memcpy(ba.buf, &action_def->dummy, ba.len);

  return ba;
}

byte_array_t srs_enc_ind_hdr_plain(srs_ind_hdr_t const* ind_hdr)
{
  assert(ind_hdr != NULL);

  byte_array_t ba = {0};

  ba.len = sizeof(srs_ind_hdr_t);
  ba.buf = calloc(ba.len,  sizeof(uint8_t));
  assert(ba.buf != NULL && "memory exhausted");
  memcpy(ba.buf, ind_hdr, ba.len);

  return ba;
}

byte_array_t srs_enc_ind_msg_plain(srs_ind_msg_t const* ind_msg) { assert(ind_msg != NULL);
  // compute the total size of the serialized buffer
  uint32_t len = sizeof(ind_msg->len); 
  for(uint32_t i = 0; i< ind_msg->len; i++){
    const srs_indication_stats_impl_t* srs_stats = &ind_msg->indication_stats[i];
    len += sizeof(srs_stats->ue_id);
    len += sizeof(srs_stats->srs_indication_ba.len);
    len += srs_stats->srs_indication_ba.len;
  }
  len += sizeof(ind_msg->tstamp); 
  byte_array_t ba = {0};
  ba.buf = calloc(1, len); 
  assert(ba.buf != NULL);

  memcpy(ba.buf, &ind_msg->len, sizeof(ind_msg->len));
  void* ptr = ba.buf + sizeof(ind_msg->len);

  for(uint32_t i = 0; i < ind_msg->len; ++i){
    // This won't copy the content of the packed srs_indication(it's a shallow copy)
    //memcpy(ptr, &ind_msg->indication_stats[i], sizeof(ind_msg->indication_stats[0])); 
    //ptr += sizeof(ind_msg->indication_stats[0]);

    // deep copy

    const srs_indication_stats_impl_t* srs_stats = &ind_msg->indication_stats[i];
    memcpy(ptr, &srs_stats->ue_id, sizeof(srs_stats->ue_id));
    ptr += sizeof(srs_stats->ue_id);

    memcpy(ptr, &srs_stats->srs_indication_ba.len, sizeof(srs_stats->srs_indication_ba.len));
    ptr += sizeof(srs_stats->srs_indication_ba.len);

    memcpy(ptr, srs_stats->srs_indication_ba.buf, srs_stats->srs_indication_ba.len);
    ptr += srs_stats->srs_indication_ba.len;
  }

  memcpy(ptr, &ind_msg->tstamp, sizeof(ind_msg->tstamp));
  ptr += sizeof(ind_msg->tstamp);
// cpy the byte array.
  assert(ptr == ba.buf + len && "Data layout mismatch");

  ba.len = len;
  return ba;
}


byte_array_t srs_enc_call_proc_id_plain(srs_call_proc_id_t const* call_proc_id)
{
  assert(0!=0 && "Not implemented");

  assert(call_proc_id != NULL);
  byte_array_t  ba = {0};
  return ba;
}

byte_array_t srs_enc_ctrl_hdr_plain(srs_ctrl_hdr_t const* ctrl_hdr)
{
  assert(ctrl_hdr != NULL);
  byte_array_t  ba = {0};
  ba.len = sizeof(srs_ctrl_hdr_t);
  ba.buf = calloc(ba.len ,sizeof(uint8_t)); 
  assert(ba.buf != NULL);

  memcpy(ba.buf, ctrl_hdr, ba.len);

  return ba;
}

byte_array_t srs_enc_ctrl_msg_plain(srs_ctrl_msg_t const* ctrl_msg)
{
  assert(ctrl_msg != NULL);

  byte_array_t  ba = {0};
  ba.len = sizeof(srs_ctrl_msg_t);
  ba.buf = calloc(ba.len, sizeof(uint8_t)); 
  assert(ba.buf != NULL);

  memcpy(ba.buf, ctrl_msg, ba.len);

  return ba;
}

byte_array_t srs_enc_ctrl_out_plain(srs_ctrl_out_t const* ctrl) 
{
  assert(0!=0 && "Not implemented");

  assert(ctrl != NULL );
  byte_array_t  ba = {0};
  return ba;
}

byte_array_t srs_enc_func_def_plain(srs_func_def_t const* func)
{
  assert(0!=0 && "Not implemented");

  assert(func != NULL);
  byte_array_t  ba = {0};
  return ba;
}

