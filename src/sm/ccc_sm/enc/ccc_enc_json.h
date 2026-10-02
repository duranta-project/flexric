/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */
#ifndef CCC_ENCODING_JSON_H
#define CCC_ENCODING_JSON_H

#include "../../../util/byte_array.h"
#include "../ie/ccc_data_ie.h"

typedef struct {
} ccc_enc_json_t;

byte_array_t ccc_enc_event_trigger_json(ccc_event_trigger_t const* event_trigger);

byte_array_t ccc_enc_action_def_json(ccc_action_def_t const* action_def);

byte_array_t ccc_enc_ind_hdr_json(ccc_ind_hdr_t const* ind_hdr);

byte_array_t ccc_enc_ind_msg_json(ccc_ind_msg_t const* ind_msg);

byte_array_t ccc_enc_ctrl_hdr_json(ccc_ctrl_hdr_t const* ctrl_hdr);

byte_array_t ccc_enc_ctrl_msg_json(ccc_ctrl_msg_t const* ctrl_msg);

byte_array_t ccc_enc_ctrl_out_json(ccc_ctrl_out_t const* ctrl_out);

byte_array_t ccc_enc_func_def_json(ccc_func_def_t const* func_def);

#endif