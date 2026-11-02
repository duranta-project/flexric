/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include "ran_func_def_ev_trig.h"

#include <assert.h>

void free_ran_func_def_ev_trig_dapp_sm(ran_func_def_ev_trig_dapp_sm_t* src)
{
  assert(src != NULL);
}

bool eq_ran_func_def_ev_trig_dapp_sm(ran_func_def_ev_trig_dapp_sm_t const* m0, ran_func_def_ev_trig_dapp_sm_t const* m1)
{
  if (m0 == m1)
    return true;

  if (m0 == NULL || m1 == NULL)
    return false;

  return true;
}

ran_func_def_ev_trig_dapp_sm_t cp_ran_func_def_ev_trig_dapp_sm(ran_func_def_ev_trig_dapp_sm_t const* src)
{
  assert(src != NULL);
  ran_func_def_ev_trig_dapp_sm_t dst = {};
  return dst;
}