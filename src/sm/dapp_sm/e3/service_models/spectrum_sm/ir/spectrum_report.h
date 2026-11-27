/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef SPECTRUM_SM_REPORT_IR
#define SPECTRUM_SM_REPORT_IR

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

/**
 * @brief In-memory representation of a Spectrum SM report.
 *
 * This is the IR (intermediate representation) produced by the ASN.1 decoder
 * and consumed by the xApp / controller logic.
 */
typedef struct {
  long prb_count;
  uint16_t* prbs;

  /* CLOCK_REALTIME nanoseconds, stamped by the dApp that built the report. */
  int64_t timestamp_ns;
} spectrum_sm_report_t;

/**
 * @brief Free all dynamic memory owned by a spectrum_sm_report_t.
 * The container itself is not freed, only the internal buffer.
 */
void free_spectrum_sm_report(spectrum_sm_report_t* src);

/**
 * @brief Compare two Spectrum SM reports for equality.
 *
 * @param m0  First report.
 * @param m1  Second report.
 * @return true if they are equal, false otherwise.
 */
bool eq_spectrum_sm_report(spectrum_sm_report_t const* m0, spectrum_sm_report_t const* m1);

/**
 * @brief Deep copy a Spectrum SM report.
 *
 * @param src  Source report to copy.
 * @return A deep copy of @p src.
 */
spectrum_sm_report_t cp_spectrum_sm_report(spectrum_sm_report_t const* src);

#endif
