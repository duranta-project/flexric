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

#include "../../../srs_fapi/nfapi_srs_data.h"

// typedef struct {
//     int16_t  antenna_index;
//     int32_t *cir_shifted;
//     size_t   cir_shifted_len; 
//     int16_t  peak_value;
//     int16_t  peak_index;
// } srs_cir_proc_t;


// typedef struct{
//     srs_cir_proc_t* cir_data;
//     size_t len;
// }srs_cir_proc_array_t;

// typedef struct
// {
//     int32_t *cir_shifted_concat[n_antennas];// holds num antenna pointers to cir_ant //len num_ue_ports*num_antennas, cir_shifted_len
//     int16_t *peak_values_concat; // len num_ue_ports * num_antennas
//     int16_t *peak_ind_concat; // len num_ue_ports * num_antennas
// } model_input_arrays_t;

// // add functions cp/free/ cp_to
int fill_srs_channel_array(
    const nfapi_nr_srs_normalized_channel_iq_matrix_t* channel_iq_matrix,
    const uint16_t num_ue_srs_ports, const uint16_t ofdm_symbol_size,
    c16_t srs_estimated_channel_freq[][channel_iq_matrix->num_ue_srs_ports][channel_iq_matrix->num_prgs]);


// // int srs_channel_cfr2cir(const uint16_t num_gnb_antenna_elements,
// //                                        const uint16_t num_ue_srs_ports,
// //                                        const uint16_t ofdm_symbol_size,
// //                                        c16_t srs_estimated_channel_freq[][num_ue_srs_ports][ofdm_symbol_size],
// //                                        c16_t srs_estimated_channel_time[][num_ue_srs_ports][ofdm_symbol_size],
// //                                        c16_t srs_estimated_channel_time_shifted[][num_ue_srs_ports][ofdm_symbol_size]);




// void pre_process_cir(const uint16_t num_gnb_antenna_elements,
//                     const uint16_t num_ue_srs_ports,
//                     const uint16_t ofdm_symbol_size,
//                     c16_t srs_estimated_channel_time[][num_ue_srs_ports][ofdm_symbol_size],
//                     c16_t srs_estimated_channel_time_shifted[][num_ue_srs_ports][ofdm_symbol_size],
//                     srs_cir_proc_array_t* srs_cir_concat);

#if defined(__cplusplus)
}
#endif

#endif // PROC_SRS_CH_H