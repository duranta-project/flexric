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
#include "../../examples/xApp/c/positioning/srs_fapi/nfapi_srs_data.h"
#include "../../examples/xApp/c/positioning/srs_fapi/srs_fapi_p7.h"
#include "../../src/util/time_now_us.h"

#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#define BUFFER_SIZE 1024  
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
  msg.len = 1;
  msg.tstamp = time_now_us();
  
  if(msg.len > 0 ){  
    msg.indication_stats = calloc(msg.len, sizeof(srs_indication_stats_impl_t));
    assert(msg.indication_stats != NULL && "Memory exhausted");
  }

  for(uint32_t i = 0; i < msg.len; ++i){
    srs_indication_stats_impl_t* indication_stats = &msg.indication_stats[i];
      
    // Fill dummy data in your data structure  

    indication_stats->ue_id = rand()%1000;
    /*
    indication_stats->srs_indication_ba.len = BUFFER_SIZE;
    indication_stats->srs_indication_ba.buf = calloc(BUFFER_SIZE, sizeof(uint8_t));
    for (size_t i = 0; i < BUFFER_SIZE; ++i) {
      indication_stats->srs_indication_ba.buf[i] = 1 ;//rand() % 256;  // Random byte
    }*/

    nfapi_nr_srs_indication_t *nfapi_srs_ind = calloc(1, sizeof(nfapi_nr_srs_indication_t));
    fill_srs_indication(nfapi_srs_ind);
    size_t ba_len = get_srs_indication_size(nfapi_srs_ind);
    byte_array_t ba = {.len = ba_len};
    ba.buf = malloc(ba.len);
    uint8_t *pPackedBuf = ba.buf;
    uint8_t *pWritePackedMessage    = pPackedBuf;
    uint8_t *pPackMessageEnd =  pPackedBuf + ba.len;

    printf("[RIC DEBUG INFO] pointer = %p\n",(void*)nfapi_srs_ind);
    printf("[RIC DEBUG INFO] ba initialized len: %zu bytes\n", ba.len);
    printf("[RIC DEBUG INFO] Sending SFN: %u\n", nfapi_srs_ind->sfn);
    printf("[RIC DEBUG INFO] Sending Slot: %u\n", nfapi_srs_ind->slot);
    printf("[RIC DEBUG INFO] Sending num srs: %d\n", nfapi_srs_ind->number_of_pdus);

    const uint8_t result = pack_nr_srs_indication(nfapi_srs_ind, &pWritePackedMessage, pPackMessageEnd);
    assert(result != 0 && "Error in packing SRS Indication message"); 
    size_t packedBufLen = pWritePackedMessage - pPackedBuf;// this should be eq to the ba.len

    ba.len = packedBufLen;
    printf("[RIC DEBUG INFO] ba updated len: %zu bytes\n", ba.len);
    indication_stats->srs_indication_ba = copy_byte_array(ba);

    // Clean up
    free_byte_array(ba);
    free_srs_indication(nfapi_srs_ind);
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
  ind_msg->len = 4;
  if(ind_msg->len > 0 ){  
    ind_msg->indication_stats = calloc(ind_msg->len, sizeof(srs_indication_stats_impl_t));
    assert(ind_msg->indication_stats != NULL && "Memory exhausted");
  }


  for(uint32_t i = 0; i < ind_msg->len; ++i){
    srs_indication_stats_impl_t* indication_stats = &ind_msg->indication_stats[i];
      
    // Fill dummy data in your data structure  
    indication_stats->ue_id = rand()%mod;
    printf("filled ID: %d for UE: %d\n", indication_stats->ue_id,i);
  }
}

