/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include "spectrum_control.h"

void free_spectrum_sm_control(spectrum_sm_control_t* src)
{
  assert(src != NULL);
  if (src->blockedPRBs) {
    free(src->blockedPRBs);
    src->blockedPRBs = NULL;
  }
  src->prb_count = 0;
}

bool eq_spectrum_sm_control(spectrum_sm_control_t const* m0, spectrum_sm_control_t const* m1)
{
  if (m0 == m1)
    return true;

  if (!m0 || !m1)
    return false;

  if (m0->prb_count != m1->prb_count)
    return false;

  if (m0->timestamp_ns != m1->timestamp_ns)
    return false;

  if (m0->prb_count == 0)
    return true;

  if (!m0->blockedPRBs || !m1->blockedPRBs)
    return false;

  return memcmp(m0->blockedPRBs, m1->blockedPRBs, (size_t)m0->prb_count * sizeof(uint16_t)) == 0;
}

spectrum_sm_control_t cp_spectrum_sm_control(spectrum_sm_control_t const* src)
{
  assert(src != NULL);

  spectrum_sm_control_t dst;
  dst.prb_count = src->prb_count;
  dst.blockedPRBs = NULL;
  dst.timestamp_ns = src->timestamp_ns;

  if (src->prb_count > 0 && src->blockedPRBs != NULL) {
    dst.blockedPRBs = (uint16_t*)malloc((size_t)src->prb_count * sizeof(uint16_t));
    assert(dst.blockedPRBs != NULL && "Memory exhausted in cp_ran_function_1");
    memcpy(dst.blockedPRBs, src->blockedPRBs, (size_t)src->prb_count * sizeof(uint16_t));
  }

  return dst;
}