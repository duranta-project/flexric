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

void fill_srs_ind_data(srs_ind_data_t* ind)
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

