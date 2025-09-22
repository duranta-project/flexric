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

#include <iostream>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <cstdio> // will change later to <fstream>
#include <cstddef>
#include <csignal>
#include <ctime>

#include "../../../../src/xApp/e42_xapp_api.h"
#include "../../../../src/util/time_now_us.h"
#include "../../../../src/util/byte_array.h"

#include "srs_fapi/nfapi.h"
#include "srs_fapi/nfapi_srs_data.h"
#include "srs_fapi/srs_fapi_p7.h"

#include <torch/torch.h>
#include <torch/script.h>

#include "proc_srs_channel.h"
#include "cc_inference.hpp"

//#define SRS_LOG
typedef uint32_t frame_t;
typedef uint32_t slot_t;
static  torch::jit::script::Module module;
/*static void dump_srs_report(nfapi_srs_report_tlv_t* report_tlv, const char* filename) {
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

  uint16_t total = NR_NB_SC_PER_RB* Nu * Ng * num_prgs;

  const c16_t *channel = (const c16_t*)channel_iq_matrix->channel_matrix;
  fwrite(channel, sizeof(c16_t), total, f);

  fclose(f);

  return;
}*/

void log_ric_indication(const srs_ind_msg_t* msg)
{
    srs_indication_stats_impl_t* srs_stats = msg->indication_stats;
    uint16_t ue_id = srs_stats->ue_id;
    printf("SRS UE_ID = %u\n", ue_id);

    size_t packedBufLen = srs_stats->srs_indication_ba.len;
    uint8_t *pReadPackedMessage = srs_stats->srs_indication_ba.buf;
    uint8_t *pUnpackMessageEnd = pReadPackedMessage + packedBufLen;

    nfapi_nr_srs_indication_t srs_ind = {0};
    if(unpack_nr_srs_indication(&pReadPackedMessage, pUnpackMessageEnd, &srs_ind)){
      const frame_t frame = srs_ind.sfn;
      const slot_t slot = srs_ind.slot;
      const int num_srs = srs_ind.number_of_pdus;
#ifdef SRS_LOG
      printf("xApp Unpacked SFN:%u\n", frame);
      printf("xApp Unpacked Slot:%u\n", slot);
      printf("xApp Unpacked Num of SRS PDUs:%d\n", num_srs);
#endif
      nfapi_nr_srs_indication_pdu_t *srs_list = srs_ind.pdu_list;
      for (int i = 0; i < num_srs; i++) {
        nfapi_nr_srs_indication_pdu_t *srs_ind_pdu = &srs_list[i];
#ifdef SRS_LOG
        printf("xApp Unpacked RNTI:%u\n", srs_ind_pdu->rnti);
        printf("xApp Unpacked TA Offset:%u\n", srs_ind_pdu->timing_advance_offset);
        printf("xApp Unpacked TA Offset nsec:%u\n", srs_ind_pdu->timing_advance_offset_nsec);
        printf("xApp Unpacked SRS Usage:%u\n", srs_ind_pdu->srs_usage);
        printf("xApp Unpacked Report type:%u\n", srs_ind_pdu->report_type);
#endif
        // extract the UL Channel
        nfapi_nr_srs_normalized_channel_iq_matrix_t nr_srs_channel_iq_matrix;
        unpack_nr_srs_normalized_channel_iq_matrix(&srs_ind_pdu->report_tlv.value,
                                                    srs_ind_pdu->report_tlv.length,
                                                    &nr_srs_channel_iq_matrix,
                                                    sizeof(nfapi_nr_srs_normalized_channel_iq_matrix_t));

      //Prepare inference
      c16_t srs_ch_est[N_rx][1][N_FFT];
      fill_srs_channel_array(&nr_srs_channel_iq_matrix,1,N_FFT,srs_ch_est);

      uint32_t srs_cir[N_rx][N_FFT];
      uint32_t cir_shifted[N_rx][N_SHIFT];
      uint32_t toa[N_rx];
      preprocess_cir(N_FFT, N_rx, srs_ch_est, srs_cir, cir_shifted, toa);
      std::vector<float> prediction = {0.0f, 0.0f}; // Array to store the predictions

      int result;
      result = cc_inference(module, cir_shifted, prediction);

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
    torch::Tensor tensor = torch::rand({2, 3});
    std::cout << "Random Torch tensor" << std::endl;
    std::cout << tensor << std::endl;
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
    module = load_torchscript_model("/home/bouknana/srs_data/CC_EmbeddingModel_2D_050625_torchscript.pt");
    fr_args_t args = init_fr_args(argc, argv);

    // init the xApp
    init_xapp_api(&args);
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1000ms); // wait after the xApp is initialized
    // see how many E2 nodes are connected

    e2_node_arr_xapp_t nodes = e2_nodes_xapp_api();


    assert(nodes.len > 0);

    printf("Connected E2 nodes = %d\n", nodes.len);

    // SRS REPORT handle
    sm_ans_xapp_t* srs_handle = NULL;

    if(nodes.len > 0){
        srs_handle = (sm_ans_xapp_t*)calloc( nodes.len, sizeof(sm_ans_xapp_t) );
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

    srs_sub.et = fill_srs_event_trigger();
    // problem here
    srs_sub.ad = (srs_action_def_t*)calloc(1,sizeof(srs_action_def_t));
    assert(srs_sub.ad != NULL && "Memory exhausted");
    srs_sub.ad[0] = fill_srs_action_definition();


    srs_handle[i] = report_sm_xapp_api(&nodes.n[i].id, SRS_ran_function, &srs_sub, sm_cb_srs);
    assert(srs_handle[i].success == true);
    free_srs_sub_data(&srs_sub);
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
      std::this_thread::sleep_for(1000ms);

    free_e2_node_arr_xapp(&nodes);
    std::cout << "Test C++ xApp run Successfully" << std::endl;
    return 0;
}
