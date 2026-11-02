/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef DAPP_ENCRYPTIOIN_ASN_H
#define DAPP_ENCRYPTIOIN_ASN_H

#include "../../../util/byte_array.h"
#include "../ie/dapp_data_ie.h"

typedef struct {
} dapp_enc_asn_t;

__attribute__((visibility("default"))) byte_array_t dapp_enc_event_trigger_asn(e2sm_dapp_event_trigger_t const* event_trigger);

__attribute__((visibility("default"))) byte_array_t dapp_enc_action_def_asn(e2sm_dapp_action_def_t const* action_def);

__attribute__((visibility("default"))) byte_array_t dapp_enc_ind_hdr_asn(e2sm_dapp_ind_hdr_t const*);

__attribute__((visibility("default"))) byte_array_t dapp_enc_ind_msg_asn(e2sm_dapp_ind_msg_t const*);

__attribute__((visibility("default"))) byte_array_t dapp_enc_ctrl_hdr_asn(e2sm_dapp_ctrl_hdr_t const*);

__attribute__((visibility("default"))) byte_array_t dapp_enc_ctrl_msg_asn(e2sm_dapp_ctrl_msg_t const*);

__attribute__((visibility("default"))) byte_array_t dapp_enc_ctrl_out_asn(e2sm_dapp_ctrl_out_t const*);

__attribute__((visibility("default"))) byte_array_t dapp_enc_func_def_asn(e2sm_dapp_func_def_t const*);

#endif
