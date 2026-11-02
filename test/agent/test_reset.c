/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

/*
 * E2 RESET handling in the agent.
 *
 * e2ap_handle_reset_request_agent() and e2ap_handle_reset_response_agent() were
 * assert(0) stubs. The dispatch table routes both messages to them, so once the
 * E2AP codec decodes them, a nearRT-RIC that resets its E2 nodes would take the
 * gNB down.
 */

// The asserts are the test: keep them in builds that define NDEBUG.
#undef NDEBUG

#include "agent/e2_agent.h"
#include "agent/asio_agent.h"
#include "agent/e2ap_agent.h"
#include "agent/msg_handler_agent.h"
#include "lib/e2ap/e2ap_ap_wrapper.h"
#include "lib/ind_event.h"
#include "lib/pending_events.h"
#include "util/compare.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static
int released = 0;

static
void count_release(ric_gen_id_t ric_id)
{
  (void)ric_id;
  ++released;
}

static
void free_value_test(void* key, void* value)
{
  assert(key != NULL);
  assert(value != NULL);
  free(value);
}

// The slice of e2_init_agent() the reset path touches, plus the E2AP codec to
// put the answer on the wire.
static
void init_agent_stub(e2_agent_t* ag)
{
  memset(ag, 0, sizeof(*ag));

  init_asio_agent(&ag->io);
  init_ap(&ag->ap.base.type);

  bi_map_init(&ag->pending, sizeof(int), sizeof(pending_event_t), cmp_fd, cmp_pending_event, free_value_test, free_value_test);
  bi_map_init(&ag->ind_event, sizeof(int), sizeof(ind_event_t), cmp_fd, cmp_ind_event, free_value_test, free_value_test);

  int const rc = pthread_mutex_init(&ag->mtx_ind_event, NULL);
  assert(rc == 0);

  released = 0;
}

static
void free_agent_stub(e2_agent_t* ag)
{
  bi_map_free(&ag->pending);
  bi_map_free(&ag->ind_event);
  pthread_mutex_destroy(&ag->mtx_ind_event);
}

static
void install_aperiodic_subscription(e2_agent_t* ag, int fd, uint32_t ric_req_id)
{
  ind_event_t ev = {0};
  ev.ric_id = (ric_gen_id_t){.ric_req_id = ric_req_id, .ric_inst_id = 1, .ran_func_id = 3};
  ev.fd = fd;
  ev.type = APERIODIC_SUBSCRIPTION_FLRC;
  ev.free_subs_aperiodic = count_release;

  pthread_mutex_lock(&ag->mtx_ind_event);
  bi_map_insert(&ag->ind_event, &ev.fd, sizeof(ev.fd), &ev, sizeof(ev));
  pthread_mutex_unlock(&ag->mtx_ind_event);
}

// A RESET drops every subscription and is answered on the same session: the E2
// setup stays, so no E2 SETUP REQUEST is armed.
static
void test_reset_request(void)
{
  e2_agent_t ag = {0};
  init_agent_stub(&ag);
  install_aperiodic_subscription(&ag, -1, 1);
  install_aperiodic_subscription(&ag, -2, 2);
  ag.e2_session_up = true;

  e2ap_msg_t msg = {.type = E2AP_RESET_REQUEST};
#if defined(E2AP_V2) || defined(E2AP_V3)
  msg.u_msgs.rst_req.trans_id = 7;
#endif

  e2ap_msg_t ans = e2ap_handle_reset_request_agent(&ag, &msg);
  assert(ans.type == E2AP_RESET_RESPONSE);
#if defined(E2AP_V2) || defined(E2AP_V3)
  assert(ans.u_msgs.rst_resp.trans_id == 7 && "the transaction id is echoed");
#endif

  assert(bi_map_size(&ag.ind_event) == 0 && "every subscription was dropped");
  assert(released == 2 && "the RAN was told to un-register each one");
  assert(ag.e2_session_up == true && "the E2 session stays up");
  assert(bi_map_size(&ag.pending) == 0 && "no E2 SETUP REQUEST is armed");

  // The answer goes on the wire: encode it the way the event loop does and read
  // it back.
  byte_array_t ba = e2ap_msg_enc_ag(&ag.ap, &ans);
  assert(ba.buf != NULL && ba.len > 0);

  e2ap_msg_t dec = e2ap_msg_dec_ag(&ag.ap, ba);
  assert(dec.type == E2AP_RESET_RESPONSE);
#if defined(E2AP_V2) || defined(E2AP_V3)
  assert(dec.u_msgs.rst_resp.trans_id == 7);
#endif

  e2ap_msg_free_ag(&ag.ap, &dec);
  free_byte_array(ba);
  e2ap_msg_free_ag(&ag.ap, &ans);

  free_agent_stub(&ag);
  printf("[TEST]: E2 RESET REQUEST drops the subscriptions and is answered\n");
}

// This node never sends a RESET REQUEST, so a RESET RESPONSE answers nothing.
static
void test_unsolicited_reset_response(void)
{
  e2_agent_t ag = {0};
  init_agent_stub(&ag);
  install_aperiodic_subscription(&ag, -1, 1);
  ag.e2_session_up = true;

  e2ap_msg_t const msg = {.type = E2AP_RESET_RESPONSE};

  e2ap_msg_t const ans = e2ap_handle_reset_response_agent(&ag, &msg);
  assert(ans.type == NONE_E2_MSG_TYPE);
  assert(bi_map_size(&ag.ind_event) == 1 && "the subscription is untouched");
  assert(ag.e2_session_up == true);

  stop_all_ind_events(&ag);
  free_agent_stub(&ag);
  printf("[TEST]: unsolicited E2 RESET RESPONSE ignored\n");
}

int main()
{
  test_reset_request();
  test_unsolicited_reset_response();

  printf("[TEST]: E2 RESET handling OK\n");
  return EXIT_SUCCESS;
}
