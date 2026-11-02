/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef E2SM_DAPP_INDICATION_HEADER_FORMAT_1_H
#define E2SM_DAPP_INDICATION_HEADER_FORMAT_1_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

typedef struct {
  uint32_t ran_function_id;
  uint32_t dapp_id;

  uint8_t node_type;
  uint8_t node_plmn_id[3];
  uint32_t node_nb_id;

  bool node_cu_du_id_present;
  uint64_t node_cu_du_id;

  /* Nanoseconds since the Unix epoch, from the realtime clock (time_now_ns). */
  int64_t timestamp_ns;

  /* Per-procedure correlation id assigned by the producer; 0 means none was
   * assigned, same absent convention as timestamp_ns. */
  int64_t sequence_id;

} e2sm_dapp_ind_hdr_frmt_1_t;

e2sm_dapp_ind_hdr_frmt_1_t cp_e2sm_dapp_ind_hdr_frmt_1(e2sm_dapp_ind_hdr_frmt_1_t const* src);

void free_e2sm_dapp_ind_hdr_frmt_1(e2sm_dapp_ind_hdr_frmt_1_t* src);

bool eq_e2sm_dapp_ind_hdr_frmt_1(e2sm_dapp_ind_hdr_frmt_1_t const* m0, e2sm_dapp_ind_hdr_frmt_1_t const* m1);

#endif
