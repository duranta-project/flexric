/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */
#pragma once

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Control Message ───────────────────────────────────────────────────── */

/* Encode a JSON control-message payload (already a valid JSON string) by
 * round-tripping it through the quicktype E2SmCccControlMessageFormat1 or
 * E2SmCccControlMessageFormat2 type.  This validates the payload against the
 * schema and re-serialises it in canonical form.
 * Returns a malloc'd buffer of *out_len bytes, or NULL on parse error. */
uint8_t* ccc_sm_encode_ctrl_msg(const char* json_payload, size_t payload_len,
                                 size_t* out_len);

/* Decode raw bytes back to a malloc'd JSON string (NUL-terminated).
 * Returns NULL on error. */
char* ccc_sm_decode_ctrl_msg(const uint8_t* data, size_t len);

/* ── Control Header ────────────────────────────────────────────────────── */

/* Encode a control header with a given RIC style type (1 or 2).
 * Returns malloc'd buffer of *out_len bytes. */
uint8_t* ccc_sm_encode_ctrl_hdr(int ric_style_type, size_t* out_len);

/* Decode raw bytes to a malloc'd JSON string (NUL-terminated). */
char* ccc_sm_decode_ctrl_hdr(const uint8_t* data, size_t len);

/* ── Indication Message ─────────────────────────────────────────────────── */

/* Encode an indication message JSON payload through the quicktype
 * E2SmCccIndicationMessageFormat1 type.
 * Returns malloc'd buffer of *out_len bytes, or NULL on error. */
uint8_t* ccc_sm_encode_ind_msg(const char* json_payload, size_t payload_len,
                                size_t* out_len);

/* Decode raw bytes to a malloc'd JSON string (NUL-terminated). */
char* ccc_sm_decode_ind_msg(const uint8_t* data, size_t len);

/* ── Indication Header ──────────────────────────────────────────────────── */

/* Encode an indication header with a timestamp and change reason string.
 * Returns malloc'd buffer of *out_len bytes. */
uint8_t* ccc_sm_encode_ind_hdr(int64_t event_time, const char* indication_reason,
                                size_t* out_len);

/* Decode raw bytes to a malloc'd JSON string (NUL-terminated). */
char* ccc_sm_decode_ind_hdr(const uint8_t* data, size_t len);

/* ── Legacy helpers (kept for backward compat) ──────────────────────────── */

/* Original functions — still available */
void* ccc_sm_pack_control(void* ctrl_msg, size_t* len_out);
void* ccc_sm_unpack_indication(const uint8_t* data, size_t len);

#ifdef __cplusplus
}
#endif
