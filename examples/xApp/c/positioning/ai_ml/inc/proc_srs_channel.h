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

#ifndef PROC_SRS_CH_H
#define PROC_SRS_CH_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "../../srs_fapi/nfapi_srs_data.h"
#include "../../common.h"


int fill_srs_channel_array(const nfapi_nr_srs_normalized_channel_iq_matrix_t* channel_iq_matrix,
                           const uint16_t num_ue_srs_ports, const uint16_t ofdm_symbol_size,
                           c16_t srs_estimated_channel_freq[][channel_iq_matrix->num_ue_srs_ports][channel_iq_matrix->num_prgs]);

void preprocess_cir(const uint16_t ofdm_symbol_size, const uint16_t num_antennas, const c16_t srs_data[num_antennas][ofdm_symbol_size],
                    uint32_t srs_cir[num_antennas][ofdm_symbol_size], uint32_t cir_shifted[num_antennas][N_SHIFT]);


#if defined(__cplusplus)
}
#endif

#endif // PROC_SRS_CH_H
