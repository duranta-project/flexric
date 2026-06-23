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

#ifndef CCC_DECRYPTION_PLAIN_H
#define CCC_DECRYPTION_PLAIN_H

#include "../../../util/byte_array.h"
#include "../ie/ccc_data_ie.h"

typedef struct {
} ccc_dec_plain_t;

ccc_event_trigger_t ccc_dec_event_trigger_plain(size_t len, uint8_t const ev_tr[len]);

ccc_action_def_t ccc_dec_action_def_plain(size_t len, uint8_t const action_def[len]);

ccc_ind_hdr_t ccc_dec_ind_hdr_plain(size_t len, uint8_t const ind_hdr[len]);

ccc_ind_msg_t ccc_dec_ind_msg_plain(size_t len, uint8_t const ind_msg[len]);

ccc_ctrl_hdr_t ccc_dec_ctrl_hdr_plain(size_t len, uint8_t const ctrl_hdr[len]);

ccc_ctrl_msg_t ccc_dec_ctrl_msg_plain(size_t len, uint8_t const ctrl_msg[len]);

ccc_ctrl_out_t ccc_dec_ctrl_out_plain(size_t len, uint8_t const ctrl_out[len]);

ccc_func_def_t ccc_dec_func_def_plain(size_t len, uint8_t const func_def[len]);

#endif

