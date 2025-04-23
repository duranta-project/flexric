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

 #ifndef FILL_RND_DATA_SRS_H
 #define FILL_RND_DATA_SRS_H
 
 #include "../../src/sm/srs_sm/ie/srs_data_ie.h"
 
 srs_ind_hdr_t fill_rnd_srs_ind_hdr(void);

 srs_ind_msg_t fill_rnd_srs_ind_msg(void);

 void fill_rnd_srs_ind_data(srs_ind_data_t* ind);

 srs_event_trigger_t fill_rnd_srs_event_trigger(void);

 srs_action_def_t fill_rnd_srs_action_definition(void);

 srs_sub_data_t fill_rnd_srs_subscription(void);

 #endif

