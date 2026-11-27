/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef E2SM_DAPP_INDICATION_MSG_FORMAT_1_H
#define E2SM_DAPP_INDICATION_MSG_FORMAT_1_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
  size_t data_size;
  uint8_t* data;
} e2sm_dapp_ind_msg_frmt_1_t;

void free_e2sm_dapp_ind_msg_frmt_1(e2sm_dapp_ind_msg_frmt_1_t* src);

bool eq_e2sm_dapp_ind_msg_frmt_1(e2sm_dapp_ind_msg_frmt_1_t const* m0, e2sm_dapp_ind_msg_frmt_1_t const* m1);

e2sm_dapp_ind_msg_frmt_1_t cp_e2sm_dapp_ind_msg_frmt_1(e2sm_dapp_ind_msg_frmt_1_t const* src);

#endif
