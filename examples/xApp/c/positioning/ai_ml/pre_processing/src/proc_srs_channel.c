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
#include "proc_srs_channel.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
int fill_srs_channel_array(const nfapi_nr_srs_normalized_channel_iq_matrix_t* channel_iq_matrix,
                           c16_t srs_estimated_channel_freq[][channel_iq_matrix->num_ue_srs_ports][channel_iq_matrix->num_prgs])
{
  // For E2, prg_size=0, so we always have subcarrier_offset=0 and n_prg = ofdm_symbol_size:

  const uint16_t step = 1;
  const uint16_t num_gnb_antenna_elements = channel_iq_matrix->num_gnb_antenna_elements;
  const uint16_t num_ue_srs_ports = channel_iq_matrix->num_ue_srs_ports;
  const uint16_t ofdm_symbol_size = channel_iq_matrix->num_prgs;
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


// int srs_channel_cfr2cir(const uint16_t num_gnb_antenna_elements,
//                                        const uint16_t num_ue_srs_ports,
//                                        const uint16_t ofdm_symbol_size,
//                                        c16_t srs_estimated_channel_freq[][num_ue_srs_ports][ofdm_symbol_size],
//                                        c16_t srs_estimated_channel_time[][num_ue_srs_ports][ofdm_symbol_size],
//                                        c16_t srs_estimated_channel_time_shifted[][num_ue_srs_ports][ofdm_symbol_size])
// {

//     for (int ant = 0; ant < num_gnb_antenna_elements; ant++) {
//         for (int p_index = 0; p_index < num_ue_srs_ports; p_index++) {
//         // Convert to time domain
//         // freq2time(ofdm_symbol_size,
//         //                 (int16_t *)srs_estimated_channel_freq[ant][p_index],
//         //                 (int16_t *)srs_estimated_channel_time[ant][p_index]);

//         memcpy(srs_estimated_channel_time_shifted[ant][p_index],
//                     &srs_estimated_channel_time[ant][p_index][ofdm_symbol_size >> 1],
//                     (ofdm_symbol_size >> 1) * sizeof(c16_t));

//         memcpy(&srs_estimated_channel_time_shifted[ant][p_index][ofdm_symbol_size >> 1],
//                     srs_estimated_channel_time[ant][p_index],
//                     (ofdm_symbol_size >> 1) * sizeof(c16_t));
//         } // for (int p_index = 0; p_index < N_ap; p_index++)
//     } // for (int ant = 0; ant < num_gnb_antenna_elements; ant++)

//     return 0;
// }


    // srs_toa_MQTT((int32_t *)srs_estimated_channel_freq[ant][0], N_symb_SRS * frame_parms->ofdm_symbol_size, gNB->Mod_id, ant, N_symb_SRS);

    //         for (int ant=0;ant<N_ant_rx;ant++){
    //         LOG_D(NR_PHY,"[first] srs_toa_ns[%d] = %d\n",ant,srs_toa_ns[ant]);
    //         if(frame_rx%10==0){
    //         srs_toa_MQTT((int32_t *)srs_estimated_channel_time[ant][0], frame_parms->ofdm_symbol_size, gNB->Mod_id, ant);

    //         }
    //         }

// ASSUME U HAVE srs_estimated_channel_freq, so create a first function: fill_srs_channel_array, there are better ways but keep it stupid for now



void pre_process_cir(const uint16_t num_gnb_antenna_elements,
                    const uint16_t num_ue_srs_ports,
                    const uint16_t ofdm_symbol_size,
                    c16_t srs_estimated_channel_time[][num_ue_srs_ports][ofdm_symbol_size],
                    c16_t srs_estimated_channel_time_shifted[][num_ue_srs_ports][ofdm_symbol_size],
                    srs_cir_proc_array_t* srs_cir_concat)
{

    for (int ant = 0; ant < num_gnb_antenna_elements; ant++) {
        for (int p_index = 0; p_index < num_ue_srs_ports; p_index++) {
        memcpy(srs_estimated_channel_time_shifted[ant][p_index],
                    &srs_estimated_channel_time[ant][p_index][ofdm_symbol_size >> 1],
                    (ofdm_symbol_size >> 1) * sizeof(c16_t));

        memcpy(&srs_estimated_channel_time_shifted[ant][p_index][ofdm_symbol_size >> 1],
                    srs_estimated_channel_time[ant][p_index],
                    (ofdm_symbol_size >> 1) * sizeof(c16_t));
        } // for (int p_index = 0; p_index < N_ap; p_index++)
    } // for (int ant = 0; ant < num_gnb_antenna_elements; ant++)

    for (int ant=0;ant<num_gnb_antenna_elements;ant++){
        srs_cir_proc_t cir_proc;

        int16_t peak_idx = 0;
        int16_t peak_val = 0;
        int32_t chest_tmp[ofdm_symbol_size];

        // Peak calculation
        int32_t max_val = 0, max_idx = 0, abs_val = 0;
        for (int k = 0; k < ofdm_symbol_size; k++) {
            int Re = ((c16_t*)srs_estimated_channel_time_shifted[ant][0])[k].r;
            int Im = ((c16_t*)srs_estimated_channel_time_shifted[ant][0])[k].i;
            abs_val = (Re * Re / 2) + (Im * Im / 2);

            if (abs_val > max_val) {
                max_val = abs_val;
                max_idx = k;
            }
            chest_tmp[k] = abs_val;  // Save to temp array

        } // peak loop

        peak_idx = max_idx;
        peak_val = max_val;
        printf("ant=%d , peak=%d\n", ant, peak_idx);
        // Circular shift of chest_tmp
        int shift = 2098; 
        int real_size = ofdm_symbol_size; 
        int32_t chest_shifted[real_size];

        for (int i = 0; i < real_size; i++) {
            chest_shifted[i] = chest_tmp[(i - shift + real_size) % real_size];
        }

        int chest_size = 100;

        // write to struct
        cir_proc.antenna_index = ant;
        cir_proc.peak_index = peak_idx;
        cir_proc.peak_value = peak_val;
        cir_proc.cir_shifted_len = chest_size;
        cir_proc.cir_shifted = malloc(cir_proc.cir_shifted_len * sizeof(*cir_proc.cir_shifted));

        memcpy(&cir_proc.cir_shifted, chest_shifted, cir_proc.cir_shifted_len * sizeof(uint32_t));
        // concatenate somehow to  another struct for all antennas, so the struct that we will feed to the model? then free?
       free(cir_proc.cir_shifted);
    } // antenna loop

}

// int main(int argc, char *argv[])
// {
   


//     return 0;
// }