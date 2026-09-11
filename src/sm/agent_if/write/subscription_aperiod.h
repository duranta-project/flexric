/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef SUBSCRIPTION_APERIODIC_MIR_H
#define SUBSCRIPTION_APERIODIC_MIR_H 

#include <stdint.h>

typedef struct {
  void (*free_aper_subs)(uint32_t ric_req_id);

  // Optional indication data (e.g., rc_ind_data_t*) to send immediately once
  // the subscription is fully registered with the agent, for report styles
  // that require sending RIC Indication upon RIC Subscription reception
  // (e.g., RC report style 5, "on demand report"). NULL if there is none.
  // Ownership passes to the agent, which forwards it exactly like any other
  // aperiodic indication.
  void* imm_ind_data;
} susbcription_aperiod_t;

#endif
