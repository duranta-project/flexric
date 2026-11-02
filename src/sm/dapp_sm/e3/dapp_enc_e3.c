/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include "dapp_enc_e3.h"

#include <assert.h>
#include <stdio.h>

#include "service_models/spectrum_sm/asn/encoder.h"

bool dapp_enc_e3_control(uint32_t ran_function_id, const dapp_e3_ctrl_payload_t* payload, uint8_t** out_buf, size_t* out_size)
{
  assert(out_buf != NULL);
  assert(out_size != NULL);

  *out_buf = NULL;
  *out_size = 0;

  if (payload == NULL)
    return false;

  switch (ran_function_id) {
    case 1: { // Spectrum RAN function ID
      if (payload->type != DAPP_E3_SM_SPECTRUM) {
        fprintf(stderr, "[DAPP][E3][ENC] Invalid spectrum payload type=%d\n", (int)payload->type);
        return false;
      }

      const spectrum_sm_control_t* ctrl = &payload->u.spectrum;

      if (ctrl->prb_count > 0 && ctrl->blockedPRBs == NULL) {
        fprintf(stderr, "[DAPP][E3][ENC] blockedPRBs == NULL but prb_count > 0\n");
        return false;
      }

      return spectrum_sm_enc_control(ctrl, out_buf, out_size);
    }

    default:
      fprintf(stderr, "[DAPP][E3][ENC] Unsupported ran_function_id=%u\n", ran_function_id);
      return false;
  }
}
