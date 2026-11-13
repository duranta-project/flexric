/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef RAN_FUNC_DEF_EV_TRIG_DAPP_SM_H
#define RAN_FUNC_DEF_EV_TRIG_DAPP_SM_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
} ran_func_def_ev_trig_dapp_sm_t;

void free_ran_func_def_ev_trig_dapp_sm(ran_func_def_ev_trig_dapp_sm_t* src);

bool eq_ran_func_def_ev_trig_dapp_sm(ran_func_def_ev_trig_dapp_sm_t const* m0, ran_func_def_ev_trig_dapp_sm_t const* m1);

ran_func_def_ev_trig_dapp_sm_t cp_ran_func_def_ev_trig_dapp_sm(ran_func_def_ev_trig_dapp_sm_t const* src);

#endif
