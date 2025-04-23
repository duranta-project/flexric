#include "sm_srs.h"
#include "../../../test/rnd/fill_rnd_data_srs.h"
#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

void init_srs_sm(void)
{
  // No data allocateds
}

void free_srs_sm(void)
{
  // No data allocated
}

bool read_srs_sm(void * data)
{
  assert(data != NULL);

  srs_ind_data_t* srs = (srs_ind_data_t*)(data);
  fill_rnd_srs_ind_data(srs);
  return true;
}

void read_srs_setup_sm(void* data)
{
  assert(data != NULL);
  assert(0 !=0 && "Not supported");
}

sm_ag_if_ans_t write_ctrl_srs_sm(void const* src)
{
  assert(src != NULL);
  assert(0 !=0 && "Not supported");
}

static
uint32_t *ric_req_id_list = NULL;
static
size_t sz_ric_req_id = 0;

static
void free_aperiodic_subscription(uint32_t ric_req_id)
{
  for (size_t i = 0; i < sz_ric_req_id; i++) {
    if (ric_req_id == ric_req_id_list[i])
      (void)ric_req_id_list[i];
  }
}

static
void* emulate_ric_ind(void* ptr)
{
 (void)ptr;
 for(size_t i = 0; i < 10; i++){
   usleep(rand()%5000);
   for(size_t j=0; j < sz_ric_req_id; j++){
     srs_ind_data_t* data = calloc(1,sizeof(srs_ind_data_t));
     assert(data != NULL && "Memory exhausted");
     data->hdr = fill_rnd_srs_ind_hdr();
     data->msg = fill_rnd_srs_ind_msg();

     async_event_agent_api(ric_req_id_list[j], data);
     printf("Event for RIC Req ID %u generated\n", ric_req_id_list[j]);
   }
 }

 return NULL;
}

static
pthread_t t_srs_subs_ctrl;

sm_ag_if_ans_t write_subs_srs_sm(void const* src)
{
  assert(src != NULL);

  wr_srs_sub_data_t* wr_srs = (wr_srs_sub_data_t*)src;

  ric_req_id_list = realloc(ric_req_id_list, (sz_ric_req_id + 1) * sizeof(uint32_t));
  (ric_req_id_list)[sz_ric_req_id] = wr_srs->ric_req_id;
  (sz_ric_req_id)++;

  int srs = pthread_create(&t_srs_subs_ctrl, NULL, emulate_ric_ind, NULL);
  assert(srs == 0);

  sm_ag_if_ans_t ans = {.type = SUBS_OUTCOME_SM_AG_IF_ANS_V0};
  ans.subs_out.type = APERIODIC_SUBSCRIPTION_FLRC;
  ans.subs_out.aper.free_aper_subs = free_aperiodic_subscription;


  return ans;
}
