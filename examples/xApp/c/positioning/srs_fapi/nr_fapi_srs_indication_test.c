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

static void fil_srs_indication_report_tlv(nfapi_srs_report_tlv_t *tlv)
{
  tlv->tag = rand16_range(0, 3);
  tlv->length = rand32_range(1, sizeof(tlv->value));
  const uint16_t last_idx = ((tlv->length + 3) / 4) - 1;
  for (int i = 0; i < last_idx; ++i) {
    tlv->value[i] = rand32();
  }
  const uint8_t num_bytes = 4 - get_tlv_padding(tlv->length);
  tlv->value[last_idx] = rand32_range(0, 1 << (8 * num_bytes));
}

static void fill_srs_indication_PDU(nfapi_nr_srs_indication_pdu_t *pdu)
{
  pdu->handle = rand32();
  pdu->rnti = rand16_range(1, 65535);
  pdu->timing_advance_offset = rand16_range(0, 63);
  pdu->timing_advance_offset_nsec = rands16_range(-16800, 16800);
  pdu->srs_usage = rand8_range(0,3);
  pdu->report_type = rand8_range(0,1);
//  fil_srs_indication_report_tlv(&pdu->report_tlv);
}

void fill_srs_indication(nfapi_nr_srs_indication_t *msg)
{
  msg->sfn = rand16_range(0, 1023);
  msg->slot = rand16_range(0, 159);
  msg->control_length = rand16(); // being ignored number_of_pdusat the moment, report is always sent inline
  msg->number_of_pdus = 1; // Minimum 1 PDUs in order to test at least one
  msg->pdu_list = calloc(msg->number_of_pdus, sizeof(*msg->pdu_list));
  for (int pdu_idx = 0; pdu_idx < msg->number_of_pdus; ++pdu_idx) {
    fill_srs_indication_PDU(&msg->pdu_list[pdu_idx]);
  }
}

