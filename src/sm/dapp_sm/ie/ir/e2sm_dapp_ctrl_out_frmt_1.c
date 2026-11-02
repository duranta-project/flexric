#include "e2sm_dapp_ctrl_out_frmt_1.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

void free_e2sm_dapp_ctrl_out_frmt_1(e2sm_dapp_ctrl_out_frmt_1_t* src)
{
  assert(src != NULL);

  if (src->e3_control_outcome != NULL) {
    free(src->e3_control_outcome);
    src->e3_control_outcome = NULL;
  }
  src->e3_control_outcome_size = 0;
}

bool eq_e2sm_dapp_ctrl_out_frmt_1(e2sm_dapp_ctrl_out_frmt_1_t const* m0, e2sm_dapp_ctrl_out_frmt_1_t const* m1)
{
  if (m0 == m1)
    return true;

  if (m0 == NULL || m1 == NULL)
    return false;

  if (m0->timestamp_ns != m1->timestamp_ns || m0->sequence_id != m1->sequence_id)
    return false;

  if (m0->e3_control_outcome_size != m1->e3_control_outcome_size)
    return false;

  if (m0->e3_control_outcome_size == 0)
    return true;

  if (m0->e3_control_outcome == NULL || m1->e3_control_outcome == NULL)
    return m0->e3_control_outcome == m1->e3_control_outcome;

  return memcmp(m0->e3_control_outcome, m1->e3_control_outcome, m0->e3_control_outcome_size) == 0;
}

e2sm_dapp_ctrl_out_frmt_1_t cp_e2sm_dapp_ctrl_out_frmt_1(e2sm_dapp_ctrl_out_frmt_1_t const* src)
{
  assert(src != NULL);

  e2sm_dapp_ctrl_out_frmt_1_t dst = {.timestamp_ns = src->timestamp_ns,
                                     .sequence_id = src->sequence_id};

  if (src->e3_control_outcome != NULL && src->e3_control_outcome_size > 0) {
    dst.e3_control_outcome = malloc(src->e3_control_outcome_size);
    assert(dst.e3_control_outcome != NULL && "Memory exhausted");
    memcpy(dst.e3_control_outcome, src->e3_control_outcome, src->e3_control_outcome_size);
    dst.e3_control_outcome_size = src->e3_control_outcome_size;
  }

  return dst;
}
