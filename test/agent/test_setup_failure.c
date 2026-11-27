/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

/*
 * An E2 SETUP FAILURE as the O-RAN SC nearRT-RIC sends it aborted the RAN
 * process.
 *
 * The message below carries a TransactionID with criticality ignore, Cause misc
 * / control-processing-overload and TimeToWait v60s. Two asserts aborted on it:
 * the decoder required criticality reject on the TransactionID, and the agent's
 * handler indexed a table of E2AP v1 cause names, in which misc is 5, with the
 * v2 cause, in which it is 6.
 */

// The asserts are the test: keep them in builds that define NDEBUG.
#undef NDEBUG

#include "agent/e2_agent.h"
#include "agent/asio_agent.h"
#include "agent/e2ap_agent.h"
#include "agent/msg_handler_agent.h"
#include "lib/e2ap/e2ap_ap_wrapper.h"
#include "lib/pending_events.h"
#include "util/compare.h"

#include <E2AP-PDU.h>

// The generated ASN.1 symbols carry the E2AP version they were built from.
#if defined(E2AP_V2)
#define E2AP_PDU_DEF asn_DEF_E2AP_PDU_e2ap_v2_03
#define E2AP_XER_DECODE xer_decode_e2ap_v2_03
#define E2AP_ENCODE_TO_BUFFER asn_encode_e2ap_v2_03_to_buffer
#elif defined(E2AP_V3)
#define E2AP_PDU_DEF asn_DEF_E2AP_PDU_e2ap_v3_01
#define E2AP_XER_DECODE xer_decode_e2ap_v3_01
#define E2AP_ENCODE_TO_BUFFER asn_encode_e2ap_v3_01_to_buffer
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static
char const osc_e2_setup_failure[] =
  "<E2AP-PDU><unsuccessfulOutcome><procedureCode>1</procedureCode><criticality><reject/></criticality>"
  "<value><E2setupFailure><protocolIEs>"
  "<E2setupFailureIEs><id>49</id><criticality><ignore/></criticality><value><TransactionID>0</TransactionID></value></E2setupFailureIEs>"
  "<E2setupFailureIEs><id>1</id><criticality><ignore/></criticality><value><Cause><misc><control-processing-overload/></misc></Cause></value></E2setupFailureIEs>"
  "<E2setupFailureIEs><id>31</id><criticality><ignore/></criticality><value><TimeToWait><v60s/></TimeToWait></value></E2setupFailureIEs>"
  "</protocolIEs></E2setupFailure></value></unsuccessfulOutcome></E2AP-PDU>";

// The message as it arrives on the wire: aligned PER, like every E2AP PDU.
static
byte_array_t osc_e2_setup_failure_aper(void)
{
  E2AP_PDU_t* pdu = NULL;
  asn_dec_rval_t const dr = E2AP_XER_DECODE(NULL, &E2AP_PDU_DEF, (void**)&pdu, osc_e2_setup_failure, strlen(osc_e2_setup_failure));
  assert(dr.code == RC_OK && "the O-RAN SC message is valid E2AP");

  byte_array_t ba = {.buf = malloc(2048), .len = 2048};
  assert(ba.buf != NULL);
  asn_enc_rval_t const er = E2AP_ENCODE_TO_BUFFER(NULL, ATS_ALIGNED_BASIC_PER, &E2AP_PDU_DEF, pdu, ba.buf, ba.len);
  assert(er.encoded > 0 && (size_t)er.encoded <= ba.len);
  ba.len = er.encoded;

  ASN_STRUCT_FREE(E2AP_PDU_DEF, pdu);
  return ba;
}

static
void free_value_test(void* key, void* value)
{
  assert(key != NULL);
  assert(value != NULL);
  free(value);
}

int main()
{
  e2_agent_t ag = {0};
  init_asio_agent(&ag.io);
  init_ap(&ag.ap.base.type);
  bi_map_init(&ag.pending, sizeof(int), sizeof(pending_event_t), cmp_fd, cmp_pending_event, free_value_test, free_value_test);

  // An E2 SETUP REQUEST is outstanding, as it is while the agent reconnects.
  int const fd_timer = create_timer_ms_asio_agent(&ag.io, 3000, 3000);
  pending_event_t const ev = SETUP_REQUEST_PENDING_EVENT;
  bi_map_insert(&ag.pending, &fd_timer, sizeof(fd_timer), &ev, sizeof(ev));
  ag.trans_id_setup_req = 1;

  byte_array_t ba = osc_e2_setup_failure_aper();
  e2ap_msg_t msg = e2ap_msg_dec_ag(&ag.ap, ba);
  free(ba.buf);

  assert(msg.type == E2_SETUP_FAILURE);
  assert(msg.u_msgs.e2_stp_fail.cause.present == CAUSE_MISC);
  assert(msg.u_msgs.e2_stp_fail.cause.misc == CAUSE_MISC_CONTROL_PROCESSING_OVERLOAD);
  assert(msg.u_msgs.e2_stp_fail.time_to_wait_ms != NULL);

  e2ap_msg_t const ans = e2ap_handle_setup_failure_agent(&ag, &msg);
  assert(ans.type == NONE_E2_MSG_TYPE);
  assert(bi_map_size(&ag.pending) == 1 && "the E2 SETUP REQUEST keeps being retried");
  assert(ag.e2_session_up == false);

  e2ap_msg_free_ag(&ag.ap, &msg);
  bi_map_free(&ag.pending);

  printf("[TEST]: O-RAN SC E2 SETUP FAILURE handled, E2 SETUP REQUEST retry kept\n");
  return EXIT_SUCCESS;
}
