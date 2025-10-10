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


static uint8_t get_tlv_padding(uint16_t tlv_length)
{
  return (4 - (tlv_length % 4)) % 4;
}

//static
int unpack_nr_srs_normalized_channel_iq_matrix(void *pMessageBuf,
                                               uint32_t messageBufLen,
                                               void *pUnpackedBuf,
                                               uint32_t unpackedBufLen)
{
  nfapi_nr_srs_normalized_channel_iq_matrix_t *nr_srs_normalized_channel_iq_matrix =
      (nfapi_nr_srs_normalized_channel_iq_matrix_t *)pUnpackedBuf;
  uint8_t *pReadPackedMessage = pMessageBuf;
  uint8_t *end = pMessageBuf + messageBufLen;

  memset(pUnpackedBuf, 0, unpackedBufLen);

  if (!(pull8(&pReadPackedMessage, &nr_srs_normalized_channel_iq_matrix->normalized_iq_representation, end)
        && pull16(&pReadPackedMessage, &nr_srs_normalized_channel_iq_matrix->num_gnb_antenna_elements, end)
        && pull16(&pReadPackedMessage, &nr_srs_normalized_channel_iq_matrix->num_ue_srs_ports, end)
        && pull16(&pReadPackedMessage, &nr_srs_normalized_channel_iq_matrix->prg_size, end)
        && pull16(&pReadPackedMessage, &nr_srs_normalized_channel_iq_matrix->num_prgs, end))) {
    return -1;
  }

  uint16_t channel_matrix_size = nr_srs_normalized_channel_iq_matrix->num_prgs
                                 * nr_srs_normalized_channel_iq_matrix->num_ue_srs_ports
                                 * nr_srs_normalized_channel_iq_matrix->num_gnb_antenna_elements;

  if (nr_srs_normalized_channel_iq_matrix->prg_size == 0){
    // (Not definde by FAPI) used for E2AP SRS-SM to send the full channel estimates to the RIC
    channel_matrix_size = NR_NB_SC_PER_RB * channel_matrix_size;
  }
  if (nr_srs_normalized_channel_iq_matrix->normalized_iq_representation == 0) {
    // 0: 16-bit normalized complex number (iqSize = 2) so multiplies the size by 2
    channel_matrix_size <<= 1;
  } else {
    // 1: 32-bit normalized complex number (iqSize = 4)
    channel_matrix_size <<= 2;
  }

  for (int i = 0; i < channel_matrix_size; i++) {
    if (!pull8(&pReadPackedMessage, &nr_srs_normalized_channel_iq_matrix->channel_matrix[i], end)) {
      return 0;
    }
  }

  return 1;
}

static uint8_t unpack_nr_srs_report_tlv_value(nfapi_srs_report_tlv_t *report_tlv, uint8_t **ppReadPackedMsg, uint8_t *end)
{
 #ifndef ENABLE_AERIAL
   for (int i = 0; i < (report_tlv->length + 3) / 4; i++) {
     if (!pull32(ppReadPackedMsg, &report_tlv->value[i], end)) {
       return 0;
     }
   }
 #else
  const uint16_t last_idx = ((report_tlv->length + 3) / 4) - 1;
  for (int i = 0; i < last_idx; i++) {
    if (!pull32(ppReadPackedMsg, &report_tlv->value[i], end)) {
      return 0;
    }
  }
  // Pull last bytes according to how much padding it would need to be 32-bit aligned
  const uint8_t padding = get_tlv_padding(report_tlv->length);
  pullx32(4 - padding, ppReadPackedMsg, &report_tlv->value[last_idx], end);
#endif
  return 1;
}


static uint8_t unpack_nr_srs_report_tlv(nfapi_srs_report_tlv_t *report_tlv, uint8_t **ppReadPackedMsg, uint8_t *end) {

  if(!(pull16(ppReadPackedMsg, &report_tlv->tag, end) &&
        pull32(ppReadPackedMsg, &report_tlv->length, end))) {
    return 0;
  }
//#ifndef ENABLE_AERIAL
  if (!unpack_nr_srs_report_tlv_value(report_tlv, ppReadPackedMsg, end)) {
    return 0;
  }
//#endif
  return 1;
}

//static
uint8_t unpack_nr_srs_indication_body(nfapi_nr_srs_indication_pdu_t *value, uint8_t **ppReadPackedMsg, uint8_t *end) {

  if(!(pull32(ppReadPackedMsg, &value->handle, end) &&
        pull16(ppReadPackedMsg, &value->rnti, end) &&
        pull16(ppReadPackedMsg, &value->timing_advance_offset, end) &&
        pulls16(ppReadPackedMsg, &value->timing_advance_offset_nsec, end) &&
        pull8(ppReadPackedMsg, &value->srs_usage, end) &&
        pull8(ppReadPackedMsg, &value->report_type, end))) {
    return 0;
  }

  if (!unpack_nr_srs_report_tlv(&value->report_tlv, ppReadPackedMsg, end)) {
    return 0;
  }

  return 1;
}


uint8_t unpack_nr_srs_indication(uint8_t **ppReadPackedMsg, uint8_t *end, void *msg) // *config = NULL
{
  nfapi_nr_srs_indication_t *pNfapiMsg = (nfapi_nr_srs_indication_t *)msg;
  if (!(pull16(ppReadPackedMsg, &pNfapiMsg->sfn, end) && pull16(ppReadPackedMsg, &pNfapiMsg->slot, end)
        && pull16(ppReadPackedMsg, &pNfapiMsg->control_length, end) && pull8(ppReadPackedMsg, &pNfapiMsg->number_of_pdus, end))) {
    return 0;
  }
  pNfapiMsg->pdu_list = calloc(pNfapiMsg->number_of_pdus, sizeof(*pNfapiMsg->pdu_list));
  for (int i = 0; i < pNfapiMsg->number_of_pdus; i++) {
    if (!unpack_nr_srs_indication_body(&pNfapiMsg->pdu_list[i], ppReadPackedMsg, end)) {
      return 0;
    }
  }

  return 1;
}

// Adapted from openairinterface5g/nfapi/oai_integration/aerial/fapi_vnf_p7.c:301
/*uint8_t aerial_unpack_nr_srs_indication(uint8_t **ppReadPackedMsg,
                                        uint8_t *end,
                                        uint8_t **pDataMsg,
                                        uint8_t *data_end,
                                        void *msg) //nfapi_p7_codec_config_t *config)
{
  uint8_t retval = unpack_nr_srs_indication(ppReadPackedMsg, end, msg);
  nfapi_nr_srs_indication_t *srs_ind = (nfapi_nr_srs_indication_t *)msg;
  for (uint8_t pdu_idx = 0; pdu_idx < srs_ind->number_of_pdus; pdu_idx++) {
    nfapi_nr_srs_indication_pdu_t *pdu = &srs_ind->pdu_list[pdu_idx];
    if (!unpack_nr_srs_report_tlv_value(&pdu->report_tlv, pDataMsg, data_end)) {
      return 0;
    }
  }
  return retval;
}*/

int fapi_nr_message_header_unpack(uint8_t **pMessageBuf,
                                  uint32_t messageBufLen,
                                  void *pUnpackedBuf,
                                  uint32_t unpackedBufLen) //,nfapi_p4_p5_codec_config_t *config)
{
  uint8_t **pReadPackedMessage = pMessageBuf;
  nfapi_nr_p4_p5_message_header_t *header = pUnpackedBuf;
  fapi_message_header_t fapi_msg = {0};

  if (pMessageBuf == NULL || pUnpackedBuf == NULL || messageBufLen < NFAPI_HEADER_LENGTH
      || unpackedBufLen < sizeof(fapi_message_header_t)) {
    return -1;
  }
  uint8_t *end = *pMessageBuf + messageBufLen;
  // process the header
  int result =
      (pull8(pReadPackedMessage, &fapi_msg.num_msg, end) && pull8(pReadPackedMessage, &fapi_msg.opaque_handle, end)
       && pull16(pReadPackedMessage, &header->message_id, end) && pull32(pReadPackedMessage, &header->message_length, end));
  return (result);
}

int fapi_nr_p7_message_unpack(void *pMessageBuf,
                              uint32_t messageBufLen,
                              void *pUnpackedBuf,
                              uint32_t unpackedBufLen)//,nfapi_p7_codec_config_t *config)
{
  int result = 0;
  nfapi_nr_p7_message_header_t *pMessageHeader = (nfapi_nr_p7_message_header_t *)pUnpackedBuf;
  fapi_message_header_t fapi_hdr;
  uint8_t *pReadPackedMessage = pMessageBuf;

  // AssertFatal(pMessageBuf != NULL && pUnpackedBuf != NULL, "P7 unpack supplied pointers are null");
  uint8_t *end = (uint8_t *)pMessageBuf + messageBufLen;
  // AssertFatal(messageBufLen >= NFAPI_HEADER_LENGTH && unpackedBufLen >= sizeof(fapi_message_header_t),
  //             "P5 unpack supplied message buffer is too small %d, %d\n",
  //             messageBufLen,
  //             unpackedBufLen);


  if (fapi_nr_message_header_unpack(&pReadPackedMessage, NFAPI_HEADER_LENGTH, &fapi_hdr, sizeof(fapi_message_header_t)) < 0) {
    // failed to read the header
    return -1;
  }
  pMessageHeader->message_length = fapi_hdr.message_length;
  pMessageHeader->message_id = fapi_hdr.message_id;
  if ((uint8_t *)(pMessageBuf + pMessageHeader->message_length) > end) {
    NFAPI_TRACE(NFAPI_TRACE_ERROR, "P7 unpack message length is greater than the message buffer \n");
    return -1;
  }

  //if (check_nr_fapi_unpack_length(NFAPI_NR_PHY_MSG_TYPE_SRS_INDICATION, unpackedBufLen)) {
  result = unpack_nr_srs_indication(&pReadPackedMessage, end, pMessageHeader);
  //}

  if (result == 0) {
    NFAPI_TRACE(NFAPI_TRACE_ERROR, "P7 Unpack failed to unpack message\n");
    return -1;
  }
  return 0;
}
// from nr_fapi_p7_utils.c
void free_srs_indication(nfapi_nr_srs_indication_t *msg)
{
  if (msg->pdu_list) {
    free(msg->pdu_list);
  }
}
