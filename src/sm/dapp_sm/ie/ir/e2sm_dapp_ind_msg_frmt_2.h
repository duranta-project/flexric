/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef E2SM_DAPP_INDICATION_MSG_FORMAT_2_H
#define E2SM_DAPP_INDICATION_MSG_FORMAT_2_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "dapp_e3_subscription_list.h"

typedef struct {
  dapp_e3_subscription_list_t dapp_e3_subs;
} e2sm_dapp_ind_msg_frmt_2_t;

void free_e2sm_dapp_ind_msg_frmt_2(e2sm_dapp_ind_msg_frmt_2_t* src);

bool eq_e2sm_dapp_ind_msg_frmt_2(e2sm_dapp_ind_msg_frmt_2_t const* m0, e2sm_dapp_ind_msg_frmt_2_t const* m1);

e2sm_dapp_ind_msg_frmt_2_t cp_e2sm_dapp_ind_msg_frmt_2(e2sm_dapp_ind_msg_frmt_2_t const* src);

#endif
