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

#include "../../../../src/xApp/e42_xapp_api.h"
#include "../../../../src/util/alg_ds/alg/defer.h"
#include "../../../../src/util/time_now_us.h"
#include "../../../../src/util/byte_array.h"

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include "srs_fapi/nfapi.h"
#include "srs_fapi/nfapi_srs_data.h"
#include "srs_fapi/srs_fapi_p7.h"

typedef uint32_t frame_t;
typedef uint32_t slot_t;

static void dump_srs_report(nfapi_srs_report_tlv_t* report_tlv, const char* filename) {
  FILE* f = fopen(filename, "w");
  if (!f) {
    perror("Failed to open file");
    return;
  }

  for (size_t i = 0; i < 16384; ++i) {
    fprintf(f, "%zu,%u\n", i, report_tlv->value[i]);
  }

  fclose(f);

  return;
}



static void dump_srs_channel_iq_matrix(nfapi_nr_srs_normalized_channel_iq_matrix_t* channel_iq_matrix, const char* filename) {
  FILE* f = fopen(filename, "wb");
  if (!f) {
    perror("Failed to open file");
    return;
  }
  uint16_t Ng = channel_iq_matrix->num_gnb_antenna_elements;
  uint16_t Nu = channel_iq_matrix->num_ue_srs_ports;
  uint16_t num_prgs = channel_iq_matrix->num_prgs;


  printf("Ng = %u\t ,Nu = %u\t, Np = %u\n", Ng, Nu, num_prgs);

  uint16_t total = Nu * Ng * num_prgs;//NR_NB_SC_PER_RB* Nu * Ng * num_prgs;

  const c16_t *channel = (const c16_t*)channel_iq_matrix->channel_matrix;
  fwrite(channel, sizeof(c16_t), total, f);

  fclose(f);

  return;
}

void log_ric_indication(const srs_ind_msg_t* msg)
{
    srs_indication_stats_impl_t* srs_stats = msg->indication_stats;
    uint16_t rnti = srs_stats->rnti;
    printf("SRS RNTI = %u\n", rnti);

    size_t packedBufLen = srs_stats->srs_unpacked_pdu.len;
    uint8_t *pReadPackedMessage = srs_stats->srs_unpacked_pdu.buf;
    uint8_t *pUnpackMessageEnd = pReadPackedMessage + packedBufLen;

    nfapi_nr_srs_indication_t srs_ind = {0};
    if(unpack_nr_srs_indication(&pReadPackedMessage, pUnpackMessageEnd, &srs_ind)){
      const frame_t frame = srs_ind.sfn;
      const slot_t slot = srs_ind.slot;
      const int num_srs = srs_ind.number_of_pdus;
      printf("xApp Unpacked SFN:%u\n", frame);
      printf("xApp Unpacked Slot:%u\n", slot);
      printf("xApp Unpacked Num of SRS PDUs:%d\n", num_srs);
      nfapi_nr_srs_indication_pdu_t *srs_list = srs_ind.pdu_list;
      for (int i = 0; i < num_srs; i++) {
        nfapi_nr_srs_indication_pdu_t *srs_ind_pdu = &srs_list[i];
        printf("xApp Unpacked RNTI:%u\n", srs_ind_pdu->rnti);
        printf("xApp Unpacked TA Offset:%u\n", srs_ind_pdu->timing_advance_offset);
        printf("xApp Unpacked TA Offset nsec:%u\n", srs_ind_pdu->timing_advance_offset_nsec);
        printf("xApp Unpacked SRS Usage:%u\n", srs_ind_pdu->srs_usage);
        printf("xApp Unpacked Report type:%u\n", srs_ind_pdu->report_type);
        dump_srs_report(&srs_ind_pdu->report_tlv, "report_tlv_xapp.csv");
        // extract the UL Channel
        nfapi_nr_srs_normalized_channel_iq_matrix_t nr_srs_channel_iq_matrix;
        unpack_nr_srs_normalized_channel_iq_matrix(&srs_ind_pdu->report_tlv.value,
                                                    srs_ind_pdu->report_tlv.length,
                                                    &nr_srs_channel_iq_matrix,
                                                    sizeof(nfapi_nr_srs_normalized_channel_iq_matrix_t));

        dump_srs_channel_iq_matrix(&nr_srs_channel_iq_matrix, "xapp_channel_rfsim.iq");

      }
    }
    free_srs_indication(&srs_ind);
}

static
uint64_t cnt_srs; // RIC indication message counter

// Call back
static
void sm_cb_srs(sm_ag_if_rd_t const* rd)
{
  assert(rd != NULL);
  assert(rd->type ==INDICATION_MSG_AGENT_IF_ANS_V0);
  assert(rd->ind.type == SRS_STATS_V0);

  int64_t now = time_now_us();
  if(true){
    printf("Received RIC indication message number: %ld\n", cnt_srs);
    printf("SRS ind_msg latency = %ld μs\n", now - rd->ind.srs.msg.tstamp);
    log_ric_indication(&rd->ind.srs.msg);

  }
  cnt_srs++;
}

static
srs_event_trigger_t fill_srs_event_trigger(void)
{
  srs_event_trigger_t et = {0};
  et.ev_trigger_cond_id = 2; // same as ind_hdr
  return et;
}

static
srs_action_def_t fill_srs_action_definition(void)
{
  srs_action_def_t ad = {0};
  ad.dummy = 1;
  return ad;
}

int main(int argc, char *argv[])
{
    fr_args_t args = init_fr_args(argc, argv);

    // init the xApp
    init_xapp_api(&args);
    sleep(1); // wait after the xApp is initialized
    // see how many E2 nodes are connected

    e2_node_arr_xapp_t nodes = e2_nodes_xapp_api();
    defer({ free_e2_node_arr_xapp(&nodes); });
  
    assert(nodes.len > 0);

    printf("Connected E2 nodes = %d\n", nodes.len);

    // SRS REPORT handle
    sm_ans_xapp_t* srs_handle = NULL;

    if(nodes.len > 0){
        srs_handle = calloc( nodes.len, sizeof(sm_ans_xapp_t) ); 
        assert(srs_handle  != NULL);
    }

    const int SRS_ran_function = 141;

    for (int i = 0; i < nodes.len; i++) {
        e2_node_connected_xapp_t* n = &nodes.n[i];
        for (size_t j = 0; j < n->len_rf; j++) {
          printf("Registered node %d ran func id = %d \n ", i, n->rf[j].id);
        }
    // SRS SM Subscription
    srs_sub_data_t srs_sub = {0};
    defer({ free_srs_sub_data(&srs_sub); });

    srs_sub.et = fill_srs_event_trigger();
    // problem here
    srs_sub.ad = calloc(1,sizeof(srs_action_def_t));
    assert(srs_sub.ad != NULL && "Memory exhausted");
    srs_sub.ad[0] = fill_srs_action_definition();


    srs_handle[i] = report_sm_xapp_api(&nodes.n[i].id, SRS_ran_function, &srs_sub, sm_cb_srs);
    assert(srs_handle[i].success == true);
    }

    sleep(10);

    // Remove the handle 
    for(int i = 0; i < nodes.len; ++i){
        if(srs_handle[i].u.handle != 0 )
          rm_report_sm_xapp_api(srs_handle[i].u.handle);
    }

    if(nodes.len > 0)
      free(srs_handle);

    //Stop the xApp
    while(try_stop_xapp_api() == false)
      usleep(1000);

    printf("Test xApp run SUCCESSFULLY\n");
}

