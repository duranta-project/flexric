/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include "dapp_dec_e3.h"
#include "service_models/spectrum_sm/asn/decoder.h"
#include <assert.h>

bool dapp_dec_e3_indication(uint32_t ran_function_id, const uint8_t* buf, size_t len, dapp_e3_ind_payload_t* out)
{
  assert(out != NULL);

  out->type = DAPP_E3_SM_NONE;

  if (buf == NULL || len == 0)
    return false;

  switch (ran_function_id) {
    case 1: // RAN Function ID 1 -> Spectrum PRB blacklist SM
      out->type = DAPP_E3_SM_SPECTRUM;
      if (!spectrum_sm_dec_report(buf, len, &out->u.spectrum)) {
        out->type = DAPP_E3_SM_NONE;
        return false;
      }
      return true;

    default:
      // unknown or unsupported RAN function
      return false;
  }
}
