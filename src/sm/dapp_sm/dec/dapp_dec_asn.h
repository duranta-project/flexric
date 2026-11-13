/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef DAPP_DECRYPTION_ASN_H
#define DAPP_DECRYPTION_ASN_H

#include <stddef.h>
#include "../ie/dapp_data_ie.h"

__attribute__((visibility("default"))) e2sm_dapp_event_trigger_t dapp_dec_event_trigger_asn(size_t len, uint8_t const ev_tr[len]);

__attribute__((visibility("default"))) e2sm_dapp_action_def_t dapp_dec_action_def_asn(size_t len, uint8_t const buf[len]);

__attribute__((visibility("default"))) e2sm_dapp_ind_hdr_t dapp_dec_ind_hdr_asn(size_t len, uint8_t const ind_hdr[len]);

__attribute__((visibility("default"))) e2sm_dapp_ind_msg_t dapp_dec_ind_msg_asn(size_t len, uint8_t const ind_msg[len]);

__attribute__((visibility("default"))) e2sm_dapp_ctrl_hdr_t dapp_dec_ctrl_hdr_asn(size_t len, uint8_t const ctrl_hdr[len]);

__attribute__((visibility("default"))) e2sm_dapp_ctrl_msg_t dapp_dec_ctrl_msg_asn(size_t len, uint8_t const ctrl_msg[len]);

__attribute__((visibility("default"))) e2sm_dapp_ctrl_out_t dapp_dec_ctrl_out_asn(size_t len, uint8_t const ctrl_out[len]);

__attribute__((visibility("default"))) e2sm_dapp_func_def_t dapp_dec_func_def_asn(size_t len, uint8_t const func_def[len]);

#endif
