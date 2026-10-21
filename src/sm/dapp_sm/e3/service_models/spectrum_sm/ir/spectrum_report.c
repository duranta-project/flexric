/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include "spectrum_report.h"

void free_spectrum_sm_report(spectrum_sm_report_t* src)
{
  assert(src != NULL);
  if (src->prbs) {
    free(src->prbs);
    src->prbs = NULL;
  }
  src->prb_count = 0;
}

bool eq_spectrum_sm_report(spectrum_sm_report_t const* m0, spectrum_sm_report_t const* m1)
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

  if (!m0->prbs || !m1->prbs)
    return false;

  return memcmp(m0->prbs, m1->prbs, (size_t)m0->prb_count * sizeof(uint16_t)) == 0;
}

spectrum_sm_report_t cp_spectrum_sm_report(spectrum_sm_report_t const* src)
{
  assert(src != NULL);

  spectrum_sm_report_t dst;
  dst.prb_count = src->prb_count;
  dst.prbs = NULL;
  dst.timestamp_ns = src->timestamp_ns;

  if (src->prb_count > 0 && src->prbs != NULL) {
    dst.prbs = (uint16_t*)malloc((size_t)src->prb_count * sizeof(uint16_t));
    assert(dst.prbs != NULL && "Memory exhausted in cp_ran_function_1");
    memcpy(dst.prbs, src->prbs, (size_t)src->prb_count * sizeof(uint16_t));
  }

  return dst;
}