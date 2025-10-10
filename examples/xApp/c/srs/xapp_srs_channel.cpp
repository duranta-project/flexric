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

#include "localization_plots/inc/cc_gui_app.h"

#define SRS_LOG

typedef uint32_t frame_t;
typedef uint32_t slot_t;

using namespace std::chrono_literals;

static ChannelApp* app = nullptr;
static pthread_mutex_t    gui_mutex   = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t    gui_ready_cond = PTHREAD_COND_INITIALIZER;
static bool              gui_ready      = false;


static int fill_srs_channel_array(const nfapi_nr_srs_normalized_channel_iq_matrix_t* channel_iq_matrix,
                           const uint16_t num_ue_srs_ports, const uint16_t num_prgs,
                           std::vector<std::vector<std::vector<c16_t>>>& srs_estimated_channel_freq)
{

  const uint16_t num_gnb_antenna_elements = channel_iq_matrix->num_gnb_antenna_elements;

  const c16_t *channel_matrix16 = (const c16_t*)channel_iq_matrix->channel_matrix;

  for (int uI = 0; uI < num_ue_srs_ports; uI++) {
    for (int gI = 0; gI < num_gnb_antenna_elements; gI++) {
      uint16_t subcarrier = 0;

      for (int pI = 0; pI < num_prgs; pI++) {
        uint16_t index = uI * num_gnb_antenna_elements * num_prgs + gI * num_prgs + pI;

        c16_t *srs_estimated_channel16 = &srs_estimated_channel_freq[gI][uI][subcarrier];

        // copy back
        srs_estimated_channel16->r = channel_matrix16[index].r;
        srs_estimated_channel16->i = channel_matrix16[index].i;

        // subcarrier increment
        subcarrier += 1;
      }
    }
  }

  return 0;
}



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
        std::cout << "xApp Unpacked SRS Usage:"<< static_cast<int>(srs_ind_pdu->srs_usage) << std::endl;
        std::cout << "xApp Unpacked Report type:"<< static_cast<int>(srs_ind_pdu->report_type) << std::endl;
#endif
        // extract the UL Channel
        nfapi_nr_srs_normalized_channel_iq_matrix_t nr_srs_channel_iq_matrix;
        unpack_nr_srs_normalized_channel_iq_matrix(&srs_ind_pdu->report_tlv.value,
                                                    srs_ind_pdu->report_tlv.length,
                                                    &nr_srs_channel_iq_matrix,
                                                    sizeof(nfapi_nr_srs_normalized_channel_iq_matrix_t));

        const uint16_t num_ue_srs_ports = nr_srs_channel_iq_matrix.num_ue_srs_ports;
        const size_t num_prgs = nr_srs_channel_iq_matrix.num_prgs;
        const size_t num_ant = nr_srs_channel_iq_matrix.num_gnb_antenna_elements;
        std::vector<std::vector<std::vector<c16_t>>> srs_est_freq(num_ant, std::vector<std::vector<c16_t>>(num_ue_srs_ports, std::vector<c16_t>(num_prgs)));
        fill_srs_channel_array(&nr_srs_channel_iq_matrix,num_ue_srs_ports,num_prgs,srs_est_freq);

        std::vector<std::vector<float>> srs_cfr(num_ant, std::vector<float>(num_prgs));
        for (size_t i = 0; i < num_ant; i++) {
          for (size_t j = 0; j < num_prgs; j++) {
            srs_cfr[i][j] = static_cast<float>(sqrt(c16amp2(srs_est_freq[i][0][j])));
          }
        }

       // Start the plot App: 1 antenna data
       // Update plot data
       pthread_mutex_lock(&gui_mutex);
       while (!gui_ready) {
          pthread_cond_wait(&gui_ready_cond, &gui_mutex);
       }
       pthread_mutex_unlock(&gui_mutex);
       app->UpdateCFR(srs_cfr);
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


static void srs_sm_report(void)
{

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

}

static void *sm_report_thread(void*)
{
  srs_sm_report();
  return nullptr;
}

static void *app_thread(void*)
{

   if(gui_ready == false){
      app = new ChannelApp("SRS Channel Plots",0,{nullptr});

      pthread_mutex_lock(&gui_mutex);
      gui_ready = true;
      pthread_cond_signal(&gui_ready_cond);
      pthread_mutex_unlock(&gui_mutex);
      app->Run();
    }
    return nullptr;
}

int main(int argc, char *argv[])
{
    fr_args_t args = init_fr_args(argc, argv);

    // init the xApp
    init_xapp_api(&args);
    std::this_thread::sleep_for(1000ms); // wait after the xApp is initialized
    // Launch xApp and GUI threads
    pthread_t app_tid, sm_thread;

    pthread_create(&sm_thread, NULL, sm_report_thread, NULL);
    pthread_create(&app_tid, NULL, app_thread, NULL);
    // Wait for thread to finish 
    pthread_join(sm_thread,NULL);
    pthread_join(app_tid,NULL);

    std::cout << "Test C++ xApp run Successfully" << std::endl;
    return 0;
}
