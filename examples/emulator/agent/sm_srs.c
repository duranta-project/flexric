#include "sm_srs.h"
#include "../../../test/rnd/fill_rnd_data_srs.h"
#include <assert.h>

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
  fill_srs_ind_data(srs);
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

