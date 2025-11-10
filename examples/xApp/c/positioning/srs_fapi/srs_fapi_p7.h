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

//  From nr_fapi_p7.h
#ifndef SRS_FAPI_P7_H
#define SRS_FAPI_P7_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "nfapi_srs_data.h"
#include "nfapi.h"
#include "debug.h"

size_t get_srs_indication_size(const nfapi_nr_srs_indication_t *msg);
uint8_t get_tlv_padding(uint16_t tlv_length);
uint8_t pack_nr_srs_indication(void *msg, uint8_t **ppWritePackedMsg, uint8_t *end);
int unpack_nr_srs_normalized_channel_iq_matrix(void *pMessageBuf,
                                               uint32_t messageBufLen,
                                               void *pUnpackedBuf,
                                               uint32_t unpackedBufLen);
uint8_t unpack_nr_srs_indication_body(nfapi_nr_srs_indication_pdu_t *value, uint8_t **ppReadPackedMsg, uint8_t *end);
uint8_t unpack_nr_srs_indication(uint8_t **ppReadPackedMsg, uint8_t *end, void *msg);
int fapi_nr_message_header_unpack(uint8_t **pMessageBuf,
                                  uint32_t messageBufLen,
                                  void *pUnpackedBuf,
                                  uint32_t unpackedBufLen);
int fapi_nr_p7_message_unpack(void *pMessageBuf,
                              uint32_t messageBufLen,
                              void *pUnpackedBuf,
                              uint32_t unpackedBufLen);

void free_srs_indication(nfapi_nr_srs_indication_t *msg);
void fill_srs_indication(nfapi_nr_srs_indication_t *msg, int sample_idx);
#if defined(__cplusplus)
}
#endif

#endif // FAPI_P7_H
