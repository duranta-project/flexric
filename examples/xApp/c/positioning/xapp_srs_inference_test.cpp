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
#include "../../../../src/util/time_now_us.h"
#include "../../../../src/util/byte_array.h"

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
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

#include <torch/torch.h>
#include <torch/script.h>

#include "cc_inference.hpp"

static  torch::jit::script::Module module;
using namespace std::chrono_literals;

void log_ric_indication(const srs_ind_msg_t* msg)
{
    srs_indication_stats_impl_t* srs_stats = msg->indication_stats;
    uint16_t ue_id = srs_stats->ue_id;
    std::cout << "SRS UE_ID =" << ue_id << std::endl;

    size_t packedBufLen = srs_stats->srs_indication_ba.len;
    uint8_t *pReadPackedMessage = srs_stats->srs_indication_ba.buf;
    std::cout << "[DEBUG INFO] ba initialized len:" << packedBufLen << "bytes" << std::endl;
    for(size_t i = 0; i < packedBufLen; i++){
      if(pReadPackedMessage[i]!=1){
      std::cout << "[DEBUG INFO] ba values: " << pReadPackedMessage[i] << "at index: " << i << std::endl;
      }
    }
    // Do the inference:
    std::vector<float> prediction = {0.0f, 0.0f}; // Array to store the predictions

    int result = dummy_cc_inference(module, prediction);
    
    std::cout << "Inference result is: " << result << std::endl;

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
    module = load_torchscript_model("/home/bouknana/srs_data/trained_models/CC_EmbeddingModel_2D_cpu.pt");
    fr_args_t args = init_fr_args(argc, argv);

    // init the xApp
    init_xapp_api(&args);
    std::this_thread::sleep_for(1000ms); // wait after the xApp is initialized
    // see how many E2 nodes are connected

    e2_node_arr_xapp_t nodes = e2_nodes_xapp_api();
  
    assert(nodes.len > 0);

    std::cout << "Connected E2 nodes = " <<  nodes.len << std::endl;

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
          std::cout << "Registered node" << i <<  "ran func id =  " << n->rf[j].id << std::endl;
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

    xapp_wait_end_api();

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

