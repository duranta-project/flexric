/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef SEQ_REPORT_STY_E2SM_DAPP_H
#define SEQ_REPORT_STY_E2SM_DAPP_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "../../../../util/byte_array.h"
#include "dapp_e3_subscription_list.h"

typedef struct {
  uint32_t report_type;
  byte_array_t name;
  uint32_t ind_hdr_type;
  uint32_t ind_msg_type;
  dapp_e3_subscription_list_t* dapp_e3_subs;
} seq_report_sty_dapp_sm_t;

void free_seq_report_sty_dapp_sm(seq_report_sty_dapp_sm_t* src);

seq_report_sty_dapp_sm_t cp_seq_report_sty_dapp_sm(seq_report_sty_dapp_sm_t const* src);

bool eq_seq_report_sty_dapp_sm(seq_report_sty_dapp_sm_t const* m0, seq_report_sty_dapp_sm_t const* m1);

#endif
