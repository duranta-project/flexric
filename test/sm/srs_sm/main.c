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


#include "../../rnd/fill_rnd_data_srs.h"
#include "../../../src/sm/srs_sm/srs_sm_agent.h"
#include "../../../src/sm/srs_sm/srs_sm_ric.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

static
srs_ind_data_t cp;

/////
// AGENT
////

static
bool read_ind_srs(void* read)
{
  assert(read != NULL);
//  assert(read->type == INDICATION_MSG_AGENT_IF_ANS_V0);
//  assert(read->ind.type == SRS_STATS_V0);

  srs_ind_data_t* ind = (srs_ind_data_t*)read; // &read->ind.srs;

  fill_srs_ind_data(ind);
  cp.hdr = cp_srs_ind_hdr(&ind->hdr);
  cp.msg = cp_srs_ind_msg(&ind->msg);
  return true;
}


static 
sm_ag_if_ans_t write_ctrl(void const* data)
{
  assert(data != NULL);

  srs_ctrl_req_data_t* ctrl = (srs_ctrl_req_data_t*)data; 
  assert(ctrl->hdr.dummy == 1);
  assert(ctrl->msg.action == 42);

  sm_ag_if_ans_t ans = {.type = CTRL_OUTCOME_SM_AG_IF_ANS_V0 };
  return ans;
}


/////////////////////////////
// Check Functions
// //////////////////////////

static
void check_eq_ran_function(sm_agent_t const* ag, sm_ric_t const* ric)
{
  assert(ag != NULL);
  assert(ric != NULL);
  assert(ag->info.id() == ric->ran_func_id);
}

// RIC -> E2
static
void check_subscription(sm_agent_t* ag, sm_ric_t* ric)
{
  assert(ag != NULL);
  assert(ric != NULL);

  char sub[] = "2_ms";
  sm_subs_data_t data = ric->proc.on_subscription(ric, &sub);

  sm_ag_if_ans_subs_t const subs = ag->proc.on_subscription(ag, &data); 
  assert(subs.type == PERIODIC_SUBSCRIPTION_FLRC);
  assert(subs.per.t.ms == 2);

  free_sm_subs_data(&data);
}

// E2 -> RIC
static
void check_indication(sm_agent_t* ag, sm_ric_t* ric)
{
  assert(ag != NULL);
  assert(ric != NULL);

  exp_ind_data_t exp = ag->proc.on_indication(ag, NULL);
  assert(exp.has_value == true);
  sm_ag_if_rd_ind_t msg = ric->proc.on_indication(ric, &exp.data);

  assert(msg.type == SRS_STATS_V0);
  srs_ind_data_t* data = &msg.srs;

  assert(eq_srs_ind_hdr(&data->hdr, &cp.hdr) == true);
  assert(eq_srs_ind_msg(&data->msg, &cp.msg) == true);
  assert(eq_srs_call_proc_id(data->proc_id, cp.proc_id) == true);

  free_srs_ind_hdr(&data->hdr);
  free_srs_ind_msg(&data->msg);

  free_exp_ind_data(&exp); 
}


// RIC -> E2
static
void check_ctrl(sm_agent_t* ag, sm_ric_t* ric)
{
  assert(ag != NULL);
  assert(ric != NULL);

  srs_ctrl_req_data_t ctrl = {.hdr.dummy = 1, .msg.action = 42}; 
  sm_ctrl_req_data_t msg = ric->proc.on_control_req(ric, &ctrl);
  
  sm_ctrl_out_data_t out = ag->proc.on_control(ag, &msg);
  assert(out.len_out == 0 && out.ctrl_out == NULL );

  free_sm_ctrl_req_data(&msg);
}

int main()
{
  sm_io_ag_ran_t io_ag = {0};
  io_ag.read_ind_tbl[SRS_STATS_V0] = read_ind_srs; 
  io_ag.write_ctrl_tbl[SRS_CTRL_REQ_V0] = write_ctrl; 

  sm_agent_t* sm_ag = make_srs_sm_agent(io_ag);
  sm_ric_t* sm_ric = make_srs_sm_ric();

  check_eq_ran_function(sm_ag, sm_ric);
  check_subscription(sm_ag, sm_ric);
  check_indication(sm_ag, sm_ric);
  check_ctrl(sm_ag, sm_ric);

  sm_ag->free_sm(sm_ag);
  sm_ric->free_sm(sm_ric);

  printf("Success\n");
  return EXIT_SUCCESS;
}

