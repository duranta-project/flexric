/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef SPECTRUM_SM_ENCODER_H
#define SPECTRUM_SM_ENCODER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "../ir/spectrum_control.h"

/**
 * @brief Encode a Spectrum SM control IR into ASN.1 PER.
 *
 * This function converts a high-level @ref spectrum_sm_control_t structure
 * into a wire-format ASN.1 PER-encoded Spectrum_PRBBlockedControl message.
 *
 * Behavior:
 *  - Validates @p out_buf and @p out_size, initializes them to NULL/0.
 *  - Returns false if @p ctrl is NULL or if ctrl->prb_count is outside
 *    the allowed [0, 273] range.
 *  - Fills a local Spectrum_Control_t ASN.1 struct:
 *      * prbCount is set to ctrl->prb_count.
 *      * blacklistedPRBsNotToBlock is allocated with
 *        prb_count * sizeof(uint16_t) bytes and filled by packing each
 *        PRB index in little-endian order (low byte, then high byte).
 *  - Uses aper_encode_to_buffer() to encode into a temporary buffer.
 *  - On encoding success:
 *      * Allocates a right-sized buffer, copies the encoded bytes there,
 *      * Stores the pointer in *out_buf and its length in *out_size.
 *  - Cleans up the ASN.1 structure with ASN_STRUCT_RESET() before returning.
 *
 * Memory ownership:
 *  - On success, the caller owns *out_buf and must free it (e.g., with free()).
 *  - On failure, *out_buf remains NULL and *out_size is 0.
 *
 * @param ctrl      Input IR control structure to encode.
 * @param out_buf   Output pointer to the newly allocated encoded buffer.
 * @param out_size  Output size of the encoded buffer, in bytes.
 *
 * @return true on successful encoding, false otherwise.
 */
bool spectrum_sm_enc_control(const spectrum_sm_control_t* ctrl, uint8_t** out_buf, size_t* out_size);

#endif
