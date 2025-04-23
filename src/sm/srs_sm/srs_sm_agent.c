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

#include "srs_sm_agent.h"

#include "../../util/alg_ds/alg/defer.h"
#include "dec/srs_dec_generic.h"
#include "srs_sm_id.h"
#include "enc/srs_enc_generic.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct{

  sm_agent_t base;

#ifdef ASN
  srs_enc_asn_t enc;
#elif FLATBUFFERS 
  srs_enc_fb_t enc;
#elif PLAIN
  srs_enc_plain_t enc;
#else
  static_assert(false, "No encryptioin type selected");
#endif

} sm_srs_agent_t;


// Function pointers provided by the RAN for the 
// 5 procedures, 
// subscription, indication, control, 
// E2 Setup and RIC Service Update. 
//
static
sm_ag_if_ans_subs_t on_subscription_srs_sm_ag(sm_agent_t const* sm_agent, const sm_subs_data_t* data)
{
  assert(sm_agent != NULL);
  assert(data != NULL);

  sm_srs_agent_t* sm = (sm_srs_agent_t*)sm_agent;

  wr_srs_sub_data_t wr_srs = {0};

  wr_srs.ric_req_id = data->ric_req_id;

  wr_srs.srs.et = srs_dec_event_trigger(&sm->enc, data->len_et, data->event_trigger);
  // Destructor
  defer({ free_srs_event_trigger(&wr_srs.srs.et); });

  wr_srs.srs.ad = malloc(sizeof(srs_action_def_t));
  assert(wr_srs.srs.ad != NULL && "Memory exhausted");
  *wr_srs.srs.ad = srs_dec_action_def(&sm->enc, data->len_ad, data->action_def);
  defer({ free_srs_action_def(wr_srs.srs.ad); });

  sm_ag_if_ans_t subs = sm->base.io.write_subs(&wr_srs);
  assert(subs.type == SUBS_OUTCOME_SM_AG_IF_ANS_V0);
  assert(subs.subs_out.type == APERIODIC_SUBSCRIPTION_FLRC);
  return subs.subs_out;
}

static
exp_ind_data_t on_indication_srs_sm_ag(sm_agent_t const* sm_agent, void* act_def)
{
  //printf("on_indication called \n");
  assert(sm_agent != NULL);
  assert(act_def != NULL && "Action definition needed for this SM");
  sm_srs_agent_t* sm = (sm_srs_agent_t*)sm_agent;

  exp_ind_data_t ret = {.has_value = true};

  // Fill Indication Header
  srs_ind_hdr_t hdr = {.dummy = 0 };
  byte_array_t ba_hdr = srs_enc_ind_hdr(&sm->enc, &hdr);
  ret.data.ind_hdr = ba_hdr.buf;
  ret.data.len_hdr = ba_hdr.len;

  // Fill Indication Message 
  //sm_ag_if_rd_t rd_if = {.type = INDICATION_MSG_AGENT_IF_ANS_V0};
  //rd_if.ind.type = SRS_STATS_V0;

  srs_ind_data_t srs = {0};
 // Liberate the memory if previously allocated by the RAN. It sucks. Profoundly
  //  defer({ free_sm_rd_if(&rd_if); }; );
  defer({ free_srs_ind_hdr(&srs.hdr) ;});
  defer({ free_srs_ind_msg(&srs.msg) ;});
  defer({ free_srs_call_proc_id(srs.proc_id);});
 
  // This may allocate memory by the RAN
  if(sm->base.io.read_ind(&srs) == false)
    return (exp_ind_data_t){.has_value = false};

  byte_array_t ba = srs_enc_ind_msg(&sm->enc, &srs.msg);
  ret.data.ind_msg = ba.buf;
  ret.data.len_msg = ba.len;

  // Fill the optional Call Process ID
  ret.data.call_process_id = NULL;
  ret.data.len_cpid = 0;

  return ret;
}

static
sm_ctrl_out_data_t on_control_srs_sm_ag(sm_agent_t const* sm_agent, sm_ctrl_req_data_t const* data)
{
  assert(sm_agent != NULL);
  assert(data != NULL);
  sm_srs_agent_t* sm = (sm_srs_agent_t*) sm_agent;

  srs_ctrl_hdr_t hdr = srs_dec_ctrl_hdr(&sm->enc, data->len_hdr, data->ctrl_hdr);
  assert(hdr.dummy == 1 && "Only dummy == 1 supported ");

  srs_ctrl_msg_t msg = srs_dec_ctrl_msg(&sm->enc, data->len_msg, data->ctrl_msg);
  assert(msg.dummy == 2 && "Only dummy == 2 supported");

//  sm_ag_if_wr_t wr = {.type = CONTROL_SM_AG_IF_WR };
//  wr.ctrl.type = SRS_CTRL_REQ_V0; 

  srs_ctrl_req_data_t srs_ctrl = {0};
  srs_ctrl.hdr.dummy = hdr.dummy;
  srs_ctrl.msg.dummy = msg.dummy;

  sm->base.io.write_ctrl(&srs_ctrl);

// Answer from the E2 Node
  sm_ctrl_out_data_t ret = {0};
  ret.len_out = 0;
  ret.ctrl_out = NULL;

  printf("on_control called \n");
  return ret;
}

static
sm_e2_setup_data_t on_e2_setup_srs_sm_ag(sm_agent_t const* sm_agent)
{
  assert(sm_agent != NULL);

  sm_srs_agent_t* sm = (sm_srs_agent_t*)sm_agent;
  (void)sm;

  sm_e2_setup_data_t setup = {.len_rfd = 0, .ran_fun_def = NULL }; 

  size_t const sz = strnlen(SM_SRS_STR, 256);
  assert(sz < 256 && "Buffer overeflow?");

  setup.len_rfd = sz;
  setup.ran_fun_def = calloc(1, sz);
  assert(setup.ran_fun_def != NULL);

  memcpy(setup.ran_fun_def, SM_SRS_STR , sz);

  return setup;
}

static
 sm_ric_service_update_data_t on_ric_service_update_srs_sm_ag(sm_agent_t const* sm_agent)
{
  assert(sm_agent != NULL);
  assert(0!=0 && "Not implemented");

  printf("on_ric_service_update called \n");
  sm_ric_service_update_data_t dst = {0}; 
  return dst;
}

static
void free_srs_sm_ag(sm_agent_t* sm_agent)
{
  assert(sm_agent != NULL);
  sm_srs_agent_t* sm = (sm_srs_agent_t*)sm_agent;
  free(sm);
}


// General SM information

// Definition
static
char const* def_srs_sm_ag(void)
{
  return SM_SRS_STR;
}

// ID
static
uint16_t id_srs_sm_ag(void)
{
  return SM_SRS_ID; 
}

  // Revision
static
uint16_t rev_srs_sm_ag (void)
{
  return SM_SRS_REV;
}

// OID
static
char const* oid_srs_sm_ag (void)
{
  return SM_SRS_OID;
}




sm_agent_t* make_srs_sm_agent(sm_io_ag_ran_t io)
{
  sm_srs_agent_t* sm = calloc(1, sizeof(sm_srs_agent_t));
  assert(sm != NULL && "Memory exhausted!!!");

  // Read
  sm->base.io.read_ind = io.read_ind_tbl[SRS_STATS_V0];
  sm->base.io.read_setup = io.read_setup_tbl[SRS_AGENT_IF_E2_SETUP_ANS_V0];
 
  //Write
  sm->base.io.write_ctrl = io.write_ctrl_tbl[SRS_CTRL_REQ_V0];
  sm->base.io.write_subs = io.write_subs_tbl[SRS_SUBS_V0];

  sm->base.free_sm = free_srs_sm_ag;
  sm->base.free_act_def = NULL; //free_act_def_srs_sm_ag;

  sm->base.proc.on_subscription = on_subscription_srs_sm_ag;
  sm->base.proc.on_indication = on_indication_srs_sm_ag;
  sm->base.proc.on_control = on_control_srs_sm_ag;
  sm->base.proc.on_ric_service_update = on_ric_service_update_srs_sm_ag;
  sm->base.proc.on_e2_setup = on_e2_setup_srs_sm_ag;
  sm->base.handle = NULL;

  // General SM information
  sm->base.info.def = def_srs_sm_ag;
  sm->base.info.id =  id_srs_sm_ag;
  sm->base.info.rev = rev_srs_sm_ag;
  sm->base.info.oid = oid_srs_sm_ag;



  //*(uint16_t*)(&sm->base.ran_func_id) = SM_SRS_ID; 
  //assert(strlen( SM_SRS_STR ) < sizeof(sm->base.ran_func_name));
  //memcpy(sm->base.ran_func_name, SM_SRS_STR, strlen(SM_SRS_STR));

  return &sm->base;
}

/*
uint16_t id_srs_sm_agent(sm_agent_t const* sm_agent )
{
  assert(sm_agent != NULL);
  sm_srs_agent_t* sm = (sm_srs_agent_t*)sm_agent;
  return sm->base.ran_func_id;
}
*/

