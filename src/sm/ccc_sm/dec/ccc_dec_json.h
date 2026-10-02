/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef CCC_DECODING_JSON_H
#define CCC_DECODING_JSON_H

#include "../../../util/byte_array.h"
#include "../ie/ccc_data_ie.h"

typedef struct {
} ccc_dec_json_t;

ccc_event_trigger_t ccc_dec_event_trigger_json(size_t len, uint8_t const ev_tr[len]);

ccc_action_def_t ccc_dec_action_def_json(size_t len, uint8_t const action_def[len]);

ccc_ind_hdr_t ccc_dec_ind_hdr_json(size_t len, uint8_t const ind_hdr[len]);

ccc_ind_msg_t ccc_dec_ind_msg_json(size_t len, uint8_t const ind_msg[len]);

ccc_ctrl_hdr_t ccc_dec_ctrl_hdr_json(size_t len, uint8_t const ctrl_hdr[len]);

ccc_ctrl_msg_t ccc_dec_ctrl_msg_json(size_t len, uint8_t const ctrl_msg[len]);

ccc_ctrl_out_t ccc_dec_ctrl_out_json(size_t len, uint8_t const ctrl_out[len]);

ccc_func_def_t ccc_dec_func_def_json(size_t len, uint8_t const func_def[len]);

#endif