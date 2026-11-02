/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef DAPP_DEC_E3_H
#define DAPP_DEC_E3_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "../ie/dapp_data_ie.h"

/**
 * @brief Decode an E3 indication payload based on the RAN function ID.
 *
 * This function takes the raw ASN.1-encoded bytes received on the E3 interface
 * and decodes them into a typed dapp_e3_ind_payload_t structure.
 *
 * Behavior:
 *  - Initializes @p out with type DAPP_E3_SM_NONE.
 *  - Returns false immediately if the input buffer is NULL or has zero length.
 *  - Dispatches decoding based on @p ran_function_id.
 *  - If decoding fails for a supported RAN function, it resets out->type to
 *    DAPP_E3_SM_NONE and returns false.
 *  - For unknown/unsupported RAN function IDs, it returns false without
 *    modifying @p out beyond the initial DAPP_E3_SM_NONE.
 *
 * @param ran_function_id  RAN Function ID selecting which SM to decode.
 * @param buf              Pointer to raw encoded indication bytes.
 * @param len              Length of @p buf in bytes.
 * @param out              Output container for the decoded E3 payload.
 *
 * @return true on successful decode of a known SM, false otherwise.
 */
__attribute__((visibility("default"))) bool dapp_dec_e3_indication(uint32_t ran_function_id,
                                                                   const uint8_t* buf,
                                                                   size_t len,
                                                                   dapp_e3_ind_payload_t* out);

#endif
