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

#include "fill_rnd_data_srs.h"
#include "../../src/util/time_now_us.h"

#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

srs_event_trigger_t fill_rnd_srs_event_trigger(void)
{
  srs_event_trigger_t et = {0};
  et.ev_trigger_cond_id = rand()%100;
  return et;
}

srs_action_def_t fill_rnd_srs_action_definition(void)
{
  srs_action_def_t ad = {0};
  ad.dummy = rand()%100;
  return ad;
}

srs_sub_data_t fill_rnd_srs_subscription(void)
{
  srs_sub_data_t dst = {0};
  dst.et = fill_rnd_srs_event_trigger();

  dst.ad = malloc(sizeof(srs_action_def_t));
  assert(dst.ad != NULL && "Memory exhausted");
  dst.ad[0] = fill_rnd_srs_action_definition();

  return dst;
}

srs_ind_hdr_t fill_rnd_srs_ind_hdr(void)
{
  srs_ind_hdr_t hdr = {0};
  hdr.ev_trigger_cond_id = rand()%10;
  return hdr;
}

srs_ind_msg_t fill_rnd_srs_ind_msg(void)
{
  srs_ind_msg_t msg = {0};
  msg.len = rand()%4;
  msg.tstamp = time_now_us();
  
  if(msg.len > 0 ){  
    msg.indication_stats = calloc(msg.len, sizeof(srs_indication_stats_impl_t));
    assert(msg.indication_stats != NULL && "Memory exhausted");
  }

  for(uint32_t i = 0; i < msg.len; ++i){
    srs_indication_stats_impl_t* indication_stats = &msg.indication_stats[i];
      
    // Fill dummy data in your data structure  
    indication_stats->rnti=rand()%1000;
    printf("Random RNTI in fill_rnd_srs_ind_msg: %d for UE: %d\n", indication_stats->rnti,i);
  }

  return msg;
}


void fill_rnd_srs_ind_data(srs_ind_data_t* ind)
{
  assert(ind != NULL);

  srand(time(0));

  int const mod = 1024;

  // Get indication message
  srs_ind_msg_t* ind_msg = &ind->msg;
  
  // Set time now  
  ind_msg->tstamp = time_now_us();

  // Set random number of messages  
  ind_msg->len = rand()%4;
  if(ind_msg->len > 0 ){  
    ind_msg->indication_stats = calloc(ind_msg->len, sizeof(srs_indication_stats_impl_t));
    assert(ind_msg->indication_stats != NULL && "Memory exhausted");
  }


  for(uint32_t i = 0; i < ind_msg->len; ++i){
    srs_indication_stats_impl_t* indication_stats = &ind_msg->indication_stats[i];
      
    // Fill dummy data in your data structure  
    indication_stats->rnti=rand()%mod;
    printf("filled RNTI: %d for UE: %d\n", indication_stats->rnti,i);
  }
}

