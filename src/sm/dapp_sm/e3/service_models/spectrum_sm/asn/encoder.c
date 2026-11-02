/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include "encoder.h"

#include "defs/compiled_output/Spectrum-XAppControlData.h"
#include "defs/compiled_output/Spectrum-PRBBlockedControl.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "asn_application.h"
#include "per_encoder.h"

bool spectrum_sm_enc_control(const spectrum_sm_control_t* ctrl, uint8_t** out_buf, size_t* out_size)
{
  assert(out_buf != NULL);
  assert(out_size != NULL);

  *out_buf = NULL;
  *out_size = 0;

  if (ctrl == NULL)
    return false;

  if (ctrl->prb_count < 0 || ctrl->prb_count > 273)
    return false;

  Spectrum_XAppControlData_t asn;
  memset(&asn, 0, sizeof(asn));

  /* OPTIONAL: the xApp stamps this and the codec only carries it, but 0 means it
   * did not stamp -- a normal message with no producer reference, not a malformed
   * one. Encode absent rather than refusing to encode. */
  if (ctrl->timestamp_ns != 0) {
    asn.timestamp = calloc(1, sizeof(*asn.timestamp));
    assert(asn.timestamp != NULL && "Memory exhausted in spectrum_sm_enc_control");
    *asn.timestamp = (long)ctrl->timestamp_ns;
  }
  asn.controlPayload.present = Spectrum_XAppControlPayload_PR_prbBlockedControl;

  Spectrum_PRBBlockedControl_t* blocked =
      (Spectrum_PRBBlockedControl_t*)calloc(1, sizeof(*blocked));
  assert(blocked != NULL && "Memory exhausted in spectrum_sm_enc_control");
  asn.controlPayload.choice.prbBlockedControl = blocked;

  for (long i = 0; i < ctrl->prb_count; ++i) {
    const uint16_t v = ctrl->blockedPRBs[i];

    if (v > 272) {
      ASN_STRUCT_RESET(asn_DEF_Spectrum_XAppControlData, &asn);
      return false;
    }

    long* elem = (long*)calloc(1, sizeof(*elem));
    assert(elem != NULL && "Memory exhausted in spectrum_sm_enc_control");

    *elem = (long)v;

    if (ASN_SEQUENCE_ADD(&blocked->blockedPRBs.list, elem) != 0) {
      free(elem);
      ASN_STRUCT_RESET(asn_DEF_Spectrum_XAppControlData, &asn);
      return false;
    }
  }

  uint8_t tmp[2048];
  asn_enc_rval_t er = aper_encode_to_buffer(&asn_DEF_Spectrum_XAppControlData, NULL, &asn, tmp, sizeof(tmp));

  if (er.encoded <= 0) {
    ASN_STRUCT_RESET(asn_DEF_Spectrum_XAppControlData, &asn);
    return false;
  }

  const size_t encoded_bytes = (size_t)((er.encoded + 7) / 8);
  uint8_t* out = (uint8_t*)malloc(encoded_bytes);
  assert(out != NULL && "Memory exhausted in spectrum_sm_enc_control memcpy");

  memcpy(out, tmp, encoded_bytes);

  *out_buf = out;
  *out_size = encoded_bytes;

  ASN_STRUCT_RESET(asn_DEF_Spectrum_XAppControlData, &asn);
  return true;
}