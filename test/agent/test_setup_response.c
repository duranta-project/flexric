/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

/*
 * A duplicate or unsolicited E2 SETUP RESPONSE used to abort the RAN process.
 *
 * e2ap_handle_setup_response_agent() stopped the setup timer unconditionally,
 * and bi_map_extract_right() asserts that the key is in the tree. The first
 * response removed the entry, so a second one aborted in assoc_rb_tree_extract.
 * A second assert on trans_id_setup_req sat right behind it on the same path.
 */

#include "agent/e2_agent.h"
#include "agent/msg_handler_agent.h"
#include "agent/asio_agent.h"
#include "lib/pending_events.h"
#include "util/compare.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static
void free_fd_test(void* key, void* value)
{
  assert(key != NULL);
  assert(value != NULL);
  free(value);
}

static
void free_pending_ev_test(void* key, void* value)
{
  assert(key != NULL);
  assert(value != NULL);
  free(value);
}

// The slice of e2_init_agent() that the setup-response path actually touches:
// the epoll set and the pending-event map.
static
void init_agent_stub(e2_agent_t* ag)
{
  memset(ag, 0, sizeof(*ag));

  init_asio_agent(&ag->io);

  size_t const fd_sz = sizeof(int);
  size_t const event_sz = sizeof(pending_event_t);
  bi_map_init(&ag->pending, fd_sz, event_sz, cmp_fd, cmp_pending_event, free_fd_test, free_pending_ev_test);
}

static
void free_agent_stub(e2_agent_t* ag)
{
  bi_map_free(&ag->pending);
}

// What e2_start_agent() does when it sends the E2 SETUP REQUEST: arm the 3000 ms
// retry timer and record it as the outstanding pending event.
static
void arm_setup_request(e2_agent_t* ag)
{
  int const fd_timer = create_timer_ms_asio_agent(&ag->io, 3000, 3000);
  assert(fd_timer > 0);

  pending_event_t const ev = SETUP_REQUEST_PENDING_EVENT;
  bi_map_insert(&ag->pending, &fd_timer, sizeof(fd_timer), &ev, sizeof(ev));

#if defined(E2AP_V2) || defined(E2AP_V3)
  ag->trans_id_setup_req = 1;
#endif
}

static
e2ap_msg_t setup_response_msg(uint32_t trans_id)
{
  e2ap_msg_t msg = {.type = E2_SETUP_RESPONSE};
#if defined(E2AP_V2) || defined(E2AP_V3)
  msg.u_msgs.e2_stp_resp.trans_id = trans_id;
#else
  (void)trans_id;
#endif
  return msg;
}

// The reported case: the RIC answers the same setup twice.
static
void test_duplicate_setup_response(void)
{
  e2_agent_t ag = {0};
  init_agent_stub(&ag);
  arm_setup_request(&ag);

  e2ap_msg_t const msg = setup_response_msg(21);

  e2ap_msg_t const first = e2ap_handle_setup_response_agent(&ag, &msg);
  assert(first.type == NONE_E2_MSG_TYPE);
  assert(bi_map_size(&ag.pending) == 0 && "the first response consumes the pending timer");

  // Used to abort: assoc_rb_tree_extract on a key that is no longer there.
  e2ap_msg_t const second = e2ap_handle_setup_response_agent(&ag, &msg);
  assert(second.type == NONE_E2_MSG_TYPE);
  assert(bi_map_size(&ag.pending) == 0);

  free_agent_stub(&ag);
  printf("[TEST]: duplicate E2 SETUP RESPONSE ignored\n");
}

// A response to a request this node never sent.
static
void test_unsolicited_setup_response(void)
{
  e2_agent_t ag = {0};
  init_agent_stub(&ag);

  e2ap_msg_t const msg = setup_response_msg(21);

  e2ap_msg_t const ans = e2ap_handle_setup_response_agent(&ag, &msg);
  assert(ans.type == NONE_E2_MSG_TYPE);
  assert(bi_map_size(&ag.pending) == 0);

  free_agent_stub(&ag);
  printf("[TEST]: unsolicited E2 SETUP RESPONSE ignored\n");
}

// A RIC that does not echo the transaction id is still answered, since rejecting
// the mismatch would drop the only response such a RIC sends.
static
void test_mismatched_trans_id_is_accepted(void)
{
  e2_agent_t ag = {0};
  init_agent_stub(&ag);
  arm_setup_request(&ag);

  e2ap_msg_t const msg = setup_response_msg(21);

  e2ap_msg_t const ans = e2ap_handle_setup_response_agent(&ag, &msg);
  assert(ans.type == NONE_E2_MSG_TYPE);
  assert(bi_map_size(&ag.pending) == 0 && "the response was accepted, not rejected");

  free_agent_stub(&ag);
  printf("[TEST]: E2 SETUP RESPONSE with a non-echoed transaction id accepted\n");
}

int main()
{
  test_duplicate_setup_response();
  test_unsolicited_setup_response();
  test_mismatched_trans_id_is_accepted();

  printf("[TEST]: E2 SETUP RESPONSE handling OK\n");
  return EXIT_SUCCESS;
}
