/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef DAPP_ENC_E3_H
#define DAPP_ENC_E3_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "../ie/dapp_data_ie.h"

/**
 * @brief Encode an E3 control payload into a wire-format buffer.
 *
 * This function takes a typed DAPP E3 control payload and encodes it into
 * an ASN.1 wire-format buffer, dispatching to the appropriate service-model
 * encoder based on the RAN function ID.
 *
 * Behavior:
 *  - Validates @p out_buf and @p out_size, and initializes them to NULL/0.
 *  - Returns false immediately if @p payload is NULL.
 *  - Switches on @p ran_function_id.
 *  - For unsupported RAN function IDs, prints an error and returns false.
 *
 * On success:
 *  - *out_buf points to an allocated buffer containing the encoded bytes.
 *  - *out_size contains the buffer length in bytes.
 * The caller is responsible for eventually freeing *out_buf according to the
 * allocation semantics of the underlying encoder (typically free()).
 *
 * @param ran_function_id  RAN Function ID.
 * @param payload          Typed E3 control payload to encode.
 * @param out_buf          Output pointer to the encoded buffer.
 * @param out_size         Output length of the encoded buffer in bytes.
 *
 * @return true if encoding succeeds, false otherwise.
 */
__attribute__((visibility("default"))) bool dapp_enc_e3_control(uint32_t ran_function_id,
                                                                const dapp_e3_ctrl_payload_t* payload,
                                                                uint8_t** out_buf,
                                                                size_t* out_size);

#endif