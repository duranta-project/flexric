/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef SUBSCRIPTION_APERIODIC_MIR_H
#define SUBSCRIPTION_APERIODIC_MIR_H 

#include "../../../lib/e2ap/ric_gen_id_wrapper.h"

#include <stdint.h>

typedef struct {
  // Takes the whole RIC ID: the Requestor ID alone does not identify the
  // subscription being released.
  void (*free_aper_subs)(ric_gen_id_t ric_id);

  // Optional indication data (e.g., rc_ind_data_t*) to send immediately once
  // the subscription is fully registered with the agent, for report styles
  // that require sending RIC Indication upon RIC Subscription reception
  // (e.g., RC report style 5, "on demand report"). NULL if there is none.
  // Ownership passes to the agent, which forwards it exactly like any other
  // aperiodic indication.
  void* imm_ind_data;
} susbcription_aperiod_t;

#endif
