/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef SEQUENCE_CONTROL_STYLE_E2SM_DAPP_H
#define SEQUENCE_CONTROL_STYLE_E2SM_DAPP_H

#include <stdbool.h>
#include <stdlib.h>
#include "../../../../util/byte_array.h"
#include "dapp_e3_subscription_list.h"

typedef struct {
  uint32_t style_type;
  byte_array_t name;
  uint32_t hdr;
  uint32_t msg;
  uint32_t out_frmt;
  dapp_e3_subscription_list_t* dapp_e3_subs;
} seq_ctrl_style_dapp_sm_t;

void free_seq_ctrl_style_dapp_sm(seq_ctrl_style_dapp_sm_t* src);

bool eq_seq_ctrl_style_dapp_sm(seq_ctrl_style_dapp_sm_t const* m0, seq_ctrl_style_dapp_sm_t const* m1);

seq_ctrl_style_dapp_sm_t cp_seq_ctrl_style_dapp_sm(seq_ctrl_style_dapp_sm_t const* src);

#endif
