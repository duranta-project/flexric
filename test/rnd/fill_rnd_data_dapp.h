/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef FILL_RND_DATA_DAPP_H
#define FILL_RND_DATA_DAPP_H

#include "../../src/sm/dapp_sm/ie/dapp_data_ie.h"
#include "../../src/util/byte_array.h"

/* Top-level dApp SM IEs ---------------------------------------------------- */

e2sm_dapp_event_trigger_t fill_rnd_dapp_ev_trigger(void);

e2sm_dapp_action_def_t fill_rnd_dapp_action_def(void);

e2sm_dapp_ind_hdr_frmt_1_t fill_rnd_dapp_ind_hdr_frmt_1(void);
e2sm_dapp_ind_hdr_frmt_2_t fill_rnd_dapp_ind_hdr_frmt_2(void);
e2sm_dapp_ind_hdr_t fill_rnd_dapp_ind_hdr(void);

e2sm_dapp_ind_msg_frmt_1_t fill_rnd_dapp_ind_msg_frmt_1(void);
e2sm_dapp_ind_msg_frmt_2_t fill_rnd_dapp_ind_msg_frmt_2(void);
e2sm_dapp_ind_msg_t fill_rnd_dapp_ind_msg(void);

e2sm_dapp_ctrl_hdr_t fill_rnd_dapp_ctrl_hdr(void);
e2sm_dapp_ctrl_msg_t fill_rnd_dapp_ctrl_msg(void);
e2sm_dapp_ctrl_out_t fill_rnd_dapp_ctrl_out(void);

e2sm_dapp_func_def_t fill_rnd_dapp_func_def(void);

/* Top-level containers used by the agent ⇄ RIC roundtrip test -------------- */

/*
 * Indication container, randomly chooses Format 1 or Format 2.
 *   - Format 2: hdr+msg are subscription maps, e3 stays DAPP_E3_SM_NONE.
 *   - Format 1: hdr.frmt_1.ran_function_id = 1 (Spectrum), msg.frmt_1.data
 *     contains valid Spectrum-PRBBlacklistReport bytes, and e3 mirrors the
 *     same prbs so the RIC's inner E3 decoder yields a result equal to e3.
 */
dapp_ind_data_t fill_rnd_dapp_ind_data(void);

dapp_sub_data_t fill_rnd_dapp_subscription(void);

/*
 * Control request shaped to traverse the RIC's E3 spectrum encoding path:
 *   - hdr.frmt_1.ran_function_id = 1 (Spectrum)
 *   - msg.frmt_1.{data,data_size} are NULL/0 so the RIC injects encoded E3 bytes
 *   - e3.type = DAPP_E3_SM_SPECTRUM with random PRBs
 */
dapp_ctrl_req_data_t fill_dapp_ctrl(void);

e2sm_dapp_ctrl_out_t fill_dapp_ctrl_out(void);

/*
 * RAN function definition matching what the dApp SM agent installs at E2 setup.
 * Name/OID/description come from dapp_sm_id.h so the equality check after
 * encode/decode survives the agent overwriting the name in on_e2_setup.
 */
e2sm_dapp_func_def_t fill_dapp_ran_func_def(void);

/* Spectrum SM helpers ------------------------------------------------------ */

spectrum_sm_report_t fill_rnd_spectrum_report(void);
spectrum_sm_control_t fill_rnd_spectrum_control(void);

/*
 * Encode an IR Spectrum report into APER bytes via the asn1c runtime
 * directly. The production codebase only ships spectrum_sm_dec_report (the
 * gNB-side encoder lives outside this repo), so tests need this bridge to
 * round-trip the report direction.
 */
byte_array_t spectrum_report_to_aper_bytes(spectrum_sm_report_t const* r);

/*
 * Decode APER-encoded Spectrum control bytes into IR via the asn1c runtime.
 * Mirror of spectrum_report_to_aper_bytes for the control direction:
 * production only ships spectrum_sm_enc_control.
 */
bool spectrum_control_from_aper_bytes(uint8_t const* buf, size_t len, spectrum_sm_control_t* out);

#endif
