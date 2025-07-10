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
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include "../inc/proc_srs_channel.h"

int fill_srs_channel_array(const nfapi_nr_srs_normalized_channel_iq_matrix_t* channel_iq_matrix,
                           const uint16_t num_ue_srs_ports, const uint16_t ofdm_symbol_size,
                           c16_t srs_estimated_channel_freq[][channel_iq_matrix->num_ue_srs_ports][channel_iq_matrix->num_prgs])
{
  // For E2, prg_size=0, so we always have subcarrier_offset=0 and n_prg = ofdm_symbol_size:

  const uint16_t step = 1;
  const uint16_t num_gnb_antenna_elements = channel_iq_matrix->num_gnb_antenna_elements;

  const c16_t *channel_matrix16 = (const c16_t*)channel_iq_matrix->channel_matrix;
  const uint16_t num_samples = ofdm_symbol_size;

  for (int uI = 0; uI < num_ue_srs_ports; uI++) {
    for (int gI = 0; gI < num_gnb_antenna_elements; gI++) {
      uint16_t subcarrier = 0;

      for (int pI = 0; pI < num_samples; pI++) {
        uint16_t index = uI * num_gnb_antenna_elements * num_samples + gI * num_samples + pI;

        c16_t *srs_estimated_channel16 = &srs_estimated_channel_freq[gI][uI][subcarrier];

        // copy back
        srs_estimated_channel16->r = channel_matrix16[index].r;
        srs_estimated_channel16->i = channel_matrix16[index].i;

        // subcarrier increment
        subcarrier += step;
        if (subcarrier >= ofdm_symbol_size)
          subcarrier -= ofdm_symbol_size;
      }
    }
  }

  return 0;
}

/* 
computes the amplitude of the signal, truncates it and performs a right shift
*/
void preprocess_cir(const uint16_t ofdm_symbol_size, const uint16_t num_antennas, const c16_t srs_data[num_antennas][ofdm_symbol_size],
                    uint32_t srs_cir[num_antennas][ofdm_symbol_size], uint32_t cir_shifted[num_antennas][N_SHIFT])
{

  for(size_t i = 0; i < num_antennas; i++){
    for(size_t j = 0; j < ofdm_symbol_size; j++){
      srs_cir[i][j] = sqrt(c16amp2(srs_data[i][j]));
    }
  }

  uint32_t shift = 2064;

  for(size_t i = 0; i < num_antennas; i++){
    for (size_t j = 0; j < N_SHIFT; j++){
      cir_shifted[i][j] = srs_cir[i][(j + shift) % ofdm_symbol_size];
    }
  }
  return;
}

