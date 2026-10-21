/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef DAPP_E3_SPECTRUM_DECODER_H
#define DAPP_E3_SPECTRUM_DECODER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "../ir/spectrum_report.h"

/**
 * @brief Decode a Spectrum SM report from ASN.1 PER into IR.
 *
 * This function parses a PER-encoded Spectrum_Report message and fills
 * a @ref spectrum_sm_report_t intermediate representation.
 *
 * Behavior:
 *  - Returns false if @p buf is NULL or @p len == 0.
 *  - Uses aper_decode() to decode the ASN.1 Spectrum_PRBBlacklistReport_t structure.
 *  - Validates prbCount (must be >= 0).
 *  - Sets out->prb_count from asn.prbCount.
 *    * If prb_count == 0, out->prbs is left NULL and the function returns true.
 *  - For prb_count > 0:
 *      * Checks that blacklistedPRBs has at least prb_count * sizeof(uint16_t) bytes.
 *      * Allocates out->prbs with prb_count entries.
 *      * Unpacks each PRB index from little-endian (low byte, high byte).
 *  - Resets the internal ASN.1 structure with ASN_STRUCT_RESET() before returning.
 *
 * Memory ownership:
 *  - On success with prb_count > 0, the caller owns out->prbs and must free it
 *    via free_spectrum_sm_report() or an equivalent helper.
 *  - On failure, out->prb_count is set to 0 and out->prbs is NULL.
 *
 * @param buf  Pointer to the PER-encoded Spectrum-Report buffer.
 * @param len  Length of @p buf in bytes.
 * @param out  Output structure to be filled with decoded data.
 *
 * @return true on successful decode, false otherwise.
 */
bool spectrum_sm_dec_report(const uint8_t* buf, size_t len, spectrum_sm_report_t* out);

#endif
