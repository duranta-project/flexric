/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef RAN_FUNC_DEF_CTRL_DAPP_SM_H
#define RAN_FUNC_DEF_CTRL_DAPP_SM_H

#include "seq_ctrl_style.h"

#include <stdlib.h>

typedef struct {
  size_t sz_seq_ctrl_style;
  seq_ctrl_style_dapp_sm_t* seq_ctrl_style;
} ran_func_def_ctrl_dapp_sm_t;

void free_ran_func_def_ctrl_dapp_sm(ran_func_def_ctrl_dapp_sm_t* src);

bool eq_ran_func_def_ctrl_dapp_sm(ran_func_def_ctrl_dapp_sm_t const* m0, ran_func_def_ctrl_dapp_sm_t const* m1);

ran_func_def_ctrl_dapp_sm_t cp_ran_func_def_ctrl_dapp_sm(ran_func_def_ctrl_dapp_sm_t const* src);

#endif
