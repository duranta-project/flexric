/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef E2SM_DAPP_ACTION_DEFINITION_H
#define E2SM_DAPP_ACTION_DEFINITION_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
} e2sm_dapp_action_def_frmt_1_t;

void free_e2sm_dapp_action_def_frmt_1(e2sm_dapp_action_def_frmt_1_t* src);

bool eq_e2sm_dapp_action_def_frmt_1(e2sm_dapp_action_def_frmt_1_t const* m0, e2sm_dapp_action_def_frmt_1_t const* m1);

e2sm_dapp_action_def_frmt_1_t cp_e2sm_dapp_action_def_frmt_1(e2sm_dapp_action_def_frmt_1_t const* src);

#endif