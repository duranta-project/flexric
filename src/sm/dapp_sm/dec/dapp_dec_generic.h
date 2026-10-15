#ifndef DAPP_DECRYPTION_GENERIC
#define DAPP_DECRYPTION_GENERIC

#include "dapp_dec_asn.h"

#define dapp_dec_event_trigger(T, U, V) _Generic((T), dapp_enc_asn_t *: dapp_dec_event_trigger_asn)(U, V)

#define dapp_dec_action_def(T, U, V) _Generic((T), dapp_enc_asn_t *: dapp_dec_action_def_asn)(U, V)

#define dapp_dec_ind_hdr(T, U, V) _Generic((T), dapp_enc_asn_t *: dapp_dec_ind_hdr_asn)(U, V)

#define dapp_dec_ind_msg(T, U, V) _Generic((T), dapp_enc_asn_t *: dapp_dec_ind_msg_asn)(U, V)

#define dapp_dec_ctrl_hdr(T, U, V) _Generic((T), dapp_enc_asn_t *: dapp_dec_ctrl_hdr_asn)(U, V)

#define dapp_dec_ctrl_msg(T, U, V) _Generic((T), dapp_enc_asn_t *: dapp_dec_ctrl_msg_asn)(U, V)

#define dapp_dec_ctrl_out(T, U, V) _Generic((T), dapp_enc_asn_t *: dapp_dec_ctrl_out_asn)(U, V)

#define dapp_dec_func_def(T, U, V) _Generic((T), dapp_enc_asn_t *: dapp_dec_func_def_asn)(U, V)

#endif
