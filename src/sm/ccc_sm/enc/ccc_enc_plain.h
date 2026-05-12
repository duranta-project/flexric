
#ifndef CCC_ENCRYPTION_PLAIN_H
#define CCC_ENCRYPTION_PLAIN_H

#include "../../../util/byte_array.h"
#include "../ie/ccc_data_ie.h"

typedef struct {
} ccc_enc_plain_t;

byte_array_t ccc_enc_event_trigger_plain(ccc_event_trigger_t const* event_trigger);

byte_array_t ccc_enc_action_def_plain(ccc_action_def_t const* action_def);

byte_array_t ccc_enc_ind_hdr_plain(ccc_ind_hdr_t const* ind_hdr);

byte_array_t ccc_enc_ind_msg_plain(ccc_ind_msg_t const* ind_msg);

byte_array_t ccc_enc_ctrl_hdr_plain(ccc_ctrl_hdr_t const* ctrl_hdr);

byte_array_t ccc_enc_ctrl_msg_plain(ccc_ctrl_msg_t const* ctrl_msg);

byte_array_t ccc_enc_ctrl_out_plain(ccc_ctrl_out_t const* ctrl_out);

byte_array_t ccc_enc_func_def_plain(ccc_func_def_t const* func_def);

#endif

