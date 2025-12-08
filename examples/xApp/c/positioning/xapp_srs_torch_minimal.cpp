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
#include <cmath>
#include <vector>
#include <algorithm>
#include <pthread.h> 
#include "../../../../src/xApp/e42_xapp_api.h"
#include "../../../../src/util/time_now_us.h"
#include "../../../../src/util/byte_array.h"

#include "srs_fapi/nfapi.h"
#include "srs_fapi/nfapi_srs_data.h"
#include "srs_fapi/srs_fapi_p7.h"

#include "ai_ml/inc/proc_srs_channel.h"

#include <torch/torch.h>
#include <torch/script.h>

#include "proc_srs_channel.h"

#define SRS_LOG

typedef uint32_t frame_t;
typedef uint32_t slot_t;

using namespace std::chrono_literals;

// Declare torch module
static  torch::jit::script::Module module;

void log_ric_indication(const srs_ind_msg_t* msg)
{
    srs_indication_stats_impl_t* srs_stats = msg->indication_stats;
    uint16_t ue_id = srs_stats->ue_id;
    size_t packedBufLen = srs_stats->srs_indication_ba.len;
    uint8_t *pReadPackedMessage = srs_stats->srs_indication_ba.buf;
    uint8_t *pUnpackMessageEnd = pReadPackedMessage + packedBufLen;

    nfapi_nr_srs_indication_t srs_ind = {0};
    if(unpack_nr_srs_indication(&pReadPackedMessage, pUnpackMessageEnd, &srs_ind)){
      const frame_t frame = srs_ind.sfn;
      const slot_t slot = srs_ind.slot;
      const int num_srs = srs_ind.number_of_pdus;
#ifdef SRS_LOG
      std::cout << "CU-UE ID:" << ue_id << std::endl;
      std::cout << "xApp Unpacked SFN:" << frame << std::endl;
      std::cout << "xApp Unpacked Slot:"<< slot << std::endl;
      std::cout << "xApp Unpacked Num of SRS PDUs:"<< num_srs << std::endl;
#endif
      nfapi_nr_srs_indication_pdu_t *srs_list = srs_ind.pdu_list;
      for (int i = 0; i < num_srs; i++) {
        nfapi_nr_srs_indication_pdu_t *srs_ind_pdu = &srs_list[i];
#ifdef SRS_LOG
        std::cout << "xApp Unpacked RNTI:"<< srs_ind_pdu->rnti << std::endl;
        std::cout << "xApp Unpacked TA Offset:"<< srs_ind_pdu->timing_advance_offset << std::endl;
        std::cout << "xApp Unpacked TA Offset nsec:"<< srs_ind_pdu->timing_advance_offset_nsec << std::endl;
        std::cout << "xApp Unpacked SRS Usage:"<< srs_ind_pdu->srs_usage << std::endl;
        std::cout << "xApp Unpacked Report type:"<< srs_ind_pdu->report_type << std::endl;
#endif
        // extract the UL Channel
        nfapi_nr_srs_normalized_channel_iq_matrix_t nr_srs_channel_iq_matrix;
        unpack_nr_srs_normalized_channel_iq_matrix(&srs_ind_pdu->report_tlv.value,
                                                    srs_ind_pdu->report_tlv.length,
                                                    &nr_srs_channel_iq_matrix,
                                                    sizeof(nfapi_nr_srs_normalized_channel_iq_matrix_t));

        // prepare for inference
        const uint16_t num_ue_srs_ports = nr_srs_channel_iq_matrix.num_ue_srs_ports;
        const size_t ofdm_symbol_size = nr_srs_channel_iq_matrix.num_prgs;
        c16_t srs_channel_est[N_rx][N_FFT];
        fill_srs_cir(&nr_srs_channel_iq_matrix,1,N_FFT,srs_channel_est);

        // Convert to the time domain, considers 1 UE port only
        uint32_t cir_amp2[N_rx][N_FFT];
        uint32_t cir_shifted[N_rx][N_SHIFT];
        uint32_t toa[N_rx];
        preprocess_cir_h5(N_FFT, N_rx, srs_channel_est, cir_amp2, cir_shifted, toa);

       // Do the inference:
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
    std::cout << "Received RIC indication message number: " << cnt_srs << std::endl;
    std::cout << "SRS ind_msg latency = " << (now - rd->ind.srs.msg.tstamp) << "μs" << std::endl;
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
    // add code to load torscript module
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
    std::cout << "C++ xApp run Successfully" << std::endl;
    return 0;
}
