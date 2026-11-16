/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef RAN_FUNC_DEF_REPORT_DAPP_SM_H
#define RAN_FUNC_DEF_REPORT_DAPP_SM_H

#include "seq_report_sty.h"

#include <stdbool.h>
#include <stdlib.h>

typedef struct {
  size_t sz_seq_report_sty;
  seq_report_sty_dapp_sm_t* seq_report_sty;
} ran_func_def_report_dapp_sm_t;

void free_ran_func_def_report_dapp_sm(ran_func_def_report_dapp_sm_t* src);

bool eq_ran_func_def_report_dapp_sm(ran_func_def_report_dapp_sm_t const* m0, ran_func_def_report_dapp_sm_t const* m1);

ran_func_def_report_dapp_sm_t cp_ran_func_def_report_dapp_sm(ran_func_def_report_dapp_sm_t const* src);

#endif
