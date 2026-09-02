#ifndef DAPP_ENCODE_GENERIC_H
#define DAPP_ENCODE_GENERIC_H

#include "dapp_enc_asn.h"

#define dapp_enc_event_trigger(T, U) _Generic((T), dapp_enc_asn_t *: dapp_enc_event_trigger_asn)(U)

#define dapp_enc_action_def(T, U) _Generic((T), dapp_enc_asn_t *: dapp_enc_action_def_asn)(U)

#define dapp_enc_ind_hdr(T, U) _Generic((T), dapp_enc_asn_t *: dapp_enc_ind_hdr_asn)(U)

#define dapp_enc_ind_msg(T, U) _Generic((T), dapp_enc_asn_t *: dapp_enc_ind_msg_asn)(U)

#define dapp_enc_ctrl_hdr(T, U) _Generic((T), dapp_enc_asn_t *: dapp_enc_ctrl_hdr_asn)(U)

#define dapp_enc_ctrl_msg(T, U) _Generic((T), dapp_enc_asn_t *: dapp_enc_ctrl_msg_asn)(U)

#define dapp_enc_ctrl_out(T, U) _Generic((T), dapp_enc_asn_t *: dapp_enc_ctrl_out_asn)(U)

#define dapp_enc_func_def(T, U) _Generic((T), dapp_enc_asn_t *: dapp_enc_func_def_asn)(U)

#endif
