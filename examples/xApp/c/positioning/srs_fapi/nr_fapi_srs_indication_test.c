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
#include "srs_fapi_p7.h"
#include "nfapi_srs_data.h"
#include "nr_fapi_test.h"
#include "../common.h"
#include <hdf5.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

static void load_cir_sample(const char *filename, size_t sample_idx, c16_t cir[N_rx][N_FFT]) {
    // Open file and datasets
    hid_t file_id = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);
    assert(file_id >= 0 && "Failed to open HDF5 file");

    hid_t real_id = H5Dopen2(file_id, "/cir_real", H5P_DEFAULT);
    hid_t imag_id = H5Dopen2(file_id, "/cir_imag", H5P_DEFAULT);
    assert(real_id >= 0 && imag_id >= 0 && "Failed to open datasets");

    hid_t space_id = H5Dget_space(real_id);
    assert(space_id >= 0 && "Failed to get dataspace");

    // using double because dataset was created and saved with double as datatype
    double cir_real[N_rx][N_FFT];
    double cir_imag[N_rx][N_FFT];

    // Only load one sample at a time so Hyperslab selection: https://support.hdfgroup.org/documentation/hdf5/latest/_l_b_dset_sub_r_w.html
    // CIR (real and imag) are organized as N_samples x N_rx x N_FFT
    hsize_t start[3] = {sample_idx, 0, 0};
    hsize_t count[3] = {1, N_rx, N_FFT};
    H5Sselect_hyperslab(space_id, H5S_SELECT_SET, start, NULL, count, NULL);

    // Define memory space
    hsize_t mem_dims[2] = {N_rx, N_FFT};
    hid_t mem_space = H5Screate_simple(2, mem_dims, NULL);

    // Read the data
    H5Dread(real_id, H5T_NATIVE_DOUBLE, mem_space, space_id, H5P_DEFAULT, cir_real);
    H5Dread(imag_id, H5T_NATIVE_DOUBLE, mem_space, space_id, H5P_DEFAULT, cir_imag);

    // Convert to c16_t
    for(size_t ant = 0; ant < N_rx; ant++) {
        for(size_t i = 0; i < N_FFT; i++) {
            cir[ant][i].r = (int16_t)cir_real[ant][i];
            cir[ant][i].i = (int16_t)cir_imag[ant][i];
        }
    }

    // Cleanup
    H5Sclose(mem_space);
    H5Sclose(space_id);
    H5Dclose(real_id);
    H5Dclose(imag_id);
    H5Fclose(file_id);
}


static int fill_srs_channel_matrix(uint8_t *channel_matrix,
                            const uint8_t normalized_iq_representation,
                            const c16_t srs_estimated_channel_time[N_rx][N_FFT])
{
  uint64_t subcarrier_offset = 0;
  const uint16_t step = 1;

  c16_t *channel_matrix16 = (c16_t*)channel_matrix;
  c8_t *channel_matrix8 = (c8_t*)channel_matrix;

  for(int uI = 0; uI < 1; uI++) {
    for(int gI = 0; gI < N_rx; gI++) {

      uint16_t subcarrier = subcarrier_offset;
      if (subcarrier>N_FFT) {
        subcarrier -= N_FFT;
      }

      for(int pI = 0; pI < N_FFT; pI++) {
        const c16_t *srs_estimated_channel16 = srs_estimated_channel_time[gI] + subcarrier;
        uint16_t index = uI*N_rx*N_FFT + gI*N_FFT + pI;

        if (normalized_iq_representation == 0) {
          channel_matrix8[index].r = (int8_t)(srs_estimated_channel16->r>>8);
          channel_matrix8[index].i = (int8_t)(srs_estimated_channel16->i>>8);
        } else {
          channel_matrix16[index].r = srs_estimated_channel16->r;
          channel_matrix16[index].i = srs_estimated_channel16->i;
        }

        // Subcarrier increment
        subcarrier += step;
        if (subcarrier >= N_FFT) {
          subcarrier=subcarrier-N_FFT;
        }
      }
    }
  }

  return 0;
}

static int pack_nr_srs_normalized_channel_iq_matrix(void *pMessageBuf, void *pPackedBuf, uint32_t packedBufLen)
{
  nfapi_nr_srs_normalized_channel_iq_matrix_t *nr_srs_normalized_channel_iq_matrix =
      (nfapi_nr_srs_normalized_channel_iq_matrix_t *)pMessageBuf;

  uint8_t *pWritePackedMessage = pPackedBuf;
  uint8_t *end = pPackedBuf + packedBufLen;

  if (!(push8(nr_srs_normalized_channel_iq_matrix->normalized_iq_representation, &pWritePackedMessage, end)
        && push16(nr_srs_normalized_channel_iq_matrix->num_gnb_antenna_elements, &pWritePackedMessage, end)
        && push16(nr_srs_normalized_channel_iq_matrix->num_ue_srs_ports, &pWritePackedMessage, end)
        && push16(nr_srs_normalized_channel_iq_matrix->prg_size, &pWritePackedMessage, end)
        && push16(nr_srs_normalized_channel_iq_matrix->num_prgs, &pWritePackedMessage, end))) {
    return 0;
  }

  uint32_t channel_matrix_size = nr_srs_normalized_channel_iq_matrix->num_prgs
                                 * nr_srs_normalized_channel_iq_matrix->num_ue_srs_ports
                                 * nr_srs_normalized_channel_iq_matrix->num_gnb_antenna_elements;
  if (nr_srs_normalized_channel_iq_matrix->normalized_iq_representation == 0) {
    // 0: 16-bit normalized complex number (iqSize = 2)
    channel_matrix_size <<= 1;
  } else {
    // 1: 32-bit normalized complex number (iqSize = 4)
    channel_matrix_size <<= 2;
  }

  for (size_t i = 0; i < channel_matrix_size; i++) {
    if (!push8(nr_srs_normalized_channel_iq_matrix->channel_matrix[i], &pWritePackedMessage, end)) {
      return 0;
    }
  }

  // Message length
  uintptr_t msgHead = (uintptr_t)pPackedBuf;
  uintptr_t msgEnd = (uintptr_t)pWritePackedMessage;
  return (msgEnd - msgHead);
}


static void fill_srs_indication_report_tlv(nfapi_srs_report_tlv_t *tlv, int sample_idx)
{
  tlv->tag = 0;
  tlv->length = 0;
  c16_t srs_estimated_channel_time[N_rx][N_FFT];
  load_cir_sample("/home/bouknana/ric_localization_tools/playback_data/srs_cir.h5", sample_idx, srs_estimated_channel_time);
  nfapi_nr_srs_normalized_channel_iq_matrix_t nr_srs_channel_iq_matrix;
  nr_srs_channel_iq_matrix.normalized_iq_representation = 1;
  nr_srs_channel_iq_matrix.num_gnb_antenna_elements = N_rx;
  nr_srs_channel_iq_matrix.num_ue_srs_ports = 1;
  nr_srs_channel_iq_matrix.prg_size = 0;
  nr_srs_channel_iq_matrix.num_prgs = N_FFT;
  fill_srs_channel_matrix(nr_srs_channel_iq_matrix.channel_matrix,
                                    nr_srs_channel_iq_matrix.normalized_iq_representation,
                                    srs_estimated_channel_time);
  tlv->length = pack_nr_srs_normalized_channel_iq_matrix(&nr_srs_channel_iq_matrix, tlv->value, sizeof(tlv->value));
}

static void fill_srs_indication_PDU(nfapi_nr_srs_indication_pdu_t *pdu, int sample_idx)
{
  pdu->handle = rand32();
  pdu->rnti = rand16_range(1, 65535);
  pdu->timing_advance_offset = rand16_range(0, 63);
  pdu->timing_advance_offset_nsec = rands16_range(-16800, 16800);
  pdu->srs_usage = 1;
  pdu->report_type = rand8_range(0,1);
  fill_srs_indication_report_tlv(&pdu->report_tlv, sample_idx);
}

void fill_srs_indication(nfapi_nr_srs_indication_t *msg, int sample_idx)
{
  msg->sfn = rand16_range(0, 1023);
  msg->slot = rand16_range(0, 159);
  msg->control_length = rand16(); // being ignored number_of_pdusat the moment, report is always sent inline
  msg->number_of_pdus = 1; // Minimum 1 PDUs in order to test at least one
  msg->pdu_list = calloc(msg->number_of_pdus, sizeof(*msg->pdu_list));
  for (int pdu_idx = 0; pdu_idx < msg->number_of_pdus; ++pdu_idx) {
    fill_srs_indication_PDU(&msg->pdu_list[pdu_idx], sample_idx);
  }
}

