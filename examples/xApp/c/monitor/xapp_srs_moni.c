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
 
 #include <pthread.h>
 #include <stdlib.h>
 #include <stdio.h>
 #include <signal.h>
 #include <time.h>
 #include <unistd.h>

//static
//uint64_t cnt_srs; // RIC indication message counter

// Call back
static
void sm_cb_srs(sm_ag_if_rd_t const* rd)
{
  assert(rd != NULL);
  assert(rd->type ==INDICATION_MSG_AGENT_IF_ANS_V0);
  assert(rd->ind.type == SRS_STATS_V0);

  int64_t now = time_now_us();
  if(true){
    //printf("Received RIC indication message number: %d\n", cnt_srs);
    const srs_indication_stats_impl_t* srs_stats = rd->ind.srs.msg.indication_stats;
    uint16_t rnti = srs_stats->rnti;
    printf("SRS ind_msg latency = %ld μs\n", now - rd->ind.srs.msg.tstamp);
    printf("SRS RNTI = %d\n", rnti);
  }
  // cnt_srs++;
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

    // SRS RIC indication
    const char* i_0 = "5_ms"; // interval time of the RIC indication
    sm_ans_xapp_t* srs_handle = NULL;

    if(nodes.len > 0){
        srs_handle = calloc( nodes.len, sizeof(sm_ans_xapp_t) ); 
        assert(srs_handle  != NULL);
    }

    for (int i = 0; i < nodes.len; i++) {
        e2_node_connected_xapp_t* n = &nodes.n[i];
        for (size_t j = 0; j < n->len_rf; j++)
          printf("Registered node %d ran func id = %d \n ", i, n->rf[j].id);

        srs_handle[i] = report_sm_xapp_api(&nodes.n[i].id, 141, (void*)i_0, sm_cb_srs);
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

