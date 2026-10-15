/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

/*
 * Every way the SCTP association to the nearRT-RIC can end must end the E2
 * session, not only a graceful SHUTDOWN.
 *
 * notification_handle_ag() acted on SCTP_SHUTDOWN_EVENT alone, and the agent did
 * not subscribe to association events at all. A RIC that went away without a
 * SHUTDOWN (SCTP_COMM_LOST: host or network gone, or an ABORT) or that came back
 * under the same addresses (SCTP_RESTART) left every subscription installed and
 * no E2 SETUP REQUEST armed, so the RAN kept producing indications for a RIC that
 * had forgotten them and never set the E2 session up again.
 */

// The asserts are the test: keep them in builds that define NDEBUG.
#undef NDEBUG

#include "agent/e2_agent.h"
#include "agent/asio_agent.h"
#include "agent/msg_handler_agent.h"
#include "agent/not_handler_agent.h"
#include "lib/ind_event.h"
#include "lib/pending_events.h"
#include "util/compare.h"

#include <assert.h>
#include <netinet/sctp.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static
int released = 0;

// What the RAN registers as free_aper_subs: un-registers the subscription.
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

// The slice of e2_init_agent() the notification path touches: the epoll set, the
// pending-event map and the subscription map with its mutex.
static
void init_agent_stub(e2_agent_t* ag)
{
  memset(ag, 0, sizeof(*ag));

  init_asio_agent(&ag->io);

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

// What e2ap_handle_subscription_request_agent() installs for an aperiodic
// subscription, keyed by a negative synthetic fd.
static
void install_aperiodic_subscription(e2_agent_t* ag)
{
  ind_event_t ev = {0};
  ev.ric_id = (ric_gen_id_t){.ric_req_id = 1, .ric_inst_id = 1, .ran_func_id = 3};
  ev.fd = -1;
  ev.type = APERIODIC_SUBSCRIPTION_FLRC;
  ev.free_subs_aperiodic = count_release;

  pthread_mutex_lock(&ag->mtx_ind_event);
  bi_map_insert(&ag->ind_event, &ev.fd, sizeof(ev.fd), &ev, sizeof(ev));
  pthread_mutex_unlock(&ag->mtx_ind_event);
}

// What e2_start_agent() leaves behind: an E2 SETUP REQUEST retry timer.
static
void arm_setup_request(e2_agent_t* ag)
{
  int const fd_timer = create_timer_ms_asio_agent(&ag->io, 3000, 3000);
  assert(fd_timer > 0);

  pending_event_t const ev = SETUP_REQUEST_PENDING_EVENT;
  bi_map_insert(&ag->pending, &fd_timer, sizeof(fd_timer), &ev, sizeof(ev));
}

static
sctp_msg_t shutdown_event(union sctp_notification* n)
{
  memset(n, 0, sizeof(*n));
  n->sn_header.sn_type = SCTP_SHUTDOWN_EVENT;
  return (sctp_msg_t){.type = SCTP_MSG_NOTIFICATION, .notif = n};
}

static
sctp_msg_t assoc_change(union sctp_notification* n, uint16_t state)
{
  memset(n, 0, sizeof(*n));
  n->sn_header.sn_type = SCTP_ASSOC_CHANGE;
  n->sn_assoc_change.sac_state = state;
  return (sctp_msg_t){.type = SCTP_MSG_NOTIFICATION, .notif = n};
}

// The session ends: the subscription is released through the RAN's callback and
// an E2 SETUP REQUEST retry is armed to open the next one.
static
void expect_session_ended(sctp_msg_t const* msg, char const* name)
{
  e2_agent_t ag = {0};
  init_agent_stub(&ag);
  install_aperiodic_subscription(&ag);

  notification_handle_ag(&ag, msg);

  assert(bi_map_size(&ag.ind_event) == 0 && "the subscription was dropped");
  assert(released == 1 && "the RAN was told to un-register it");
  assert(bi_map_size(&ag.pending) == 1 && "an E2 SETUP REQUEST retry is armed");

  free_agent_stub(&ag);
  printf("[TEST]: %s ends the E2 session\n", name);
}

// Not an end: nothing is torn down and no retry is armed.
static
void expect_session_kept(sctp_msg_t const* msg, char const* name)
{
  e2_agent_t ag = {0};
  init_agent_stub(&ag);
  install_aperiodic_subscription(&ag);

  notification_handle_ag(&ag, msg);

  assert(bi_map_size(&ag.ind_event) == 1 && "the subscription survives");
  assert(released == 0);
  assert(bi_map_size(&ag.pending) == 0 && "no retry is armed");

  stop_all_ind_events(&ag);
  assert(released == 1);

  free_agent_stub(&ag);
  printf("[TEST]: %s leaves the E2 session alone\n", name);
}

// pend_event() asserts the pending map holds exactly one entry, so a loss while a
// setup is already outstanding -- or two losses in a row -- must not arm a second
// timer.
static
void test_one_retry_timer_only(void)
{
  e2_agent_t ag = {0};
  init_agent_stub(&ag);
  arm_setup_request(&ag);

  union sctp_notification n;
  sctp_msg_t const lost = assoc_change(&n, SCTP_COMM_LOST);

  notification_handle_ag(&ag, &lost);
  assert(bi_map_size(&ag.pending) == 1);

  notification_handle_ag(&ag, &lost);
  assert(bi_map_size(&ag.pending) == 1);

  free_agent_stub(&ag);
  printf("[TEST]: repeated losses arm a single E2 SETUP REQUEST retry\n");
}

int main()
{
  union sctp_notification n;

  sctp_msg_t msg = shutdown_event(&n);
  expect_session_ended(&msg, "SCTP_SHUTDOWN_EVENT");

  msg = assoc_change(&n, SCTP_COMM_LOST);
  expect_session_ended(&msg, "SCTP_COMM_LOST");

  msg = assoc_change(&n, SCTP_RESTART);
  expect_session_ended(&msg, "SCTP_RESTART");

  msg = assoc_change(&n, SCTP_COMM_UP);
  expect_session_kept(&msg, "SCTP_COMM_UP");

  msg = assoc_change(&n, SCTP_CANT_STR_ASSOC);
  expect_session_kept(&msg, "SCTP_CANT_STR_ASSOC");

  msg = assoc_change(&n, SCTP_SHUTDOWN_COMP);
  expect_session_kept(&msg, "SCTP_SHUTDOWN_COMP");

  test_one_retry_timer_only();

  printf("[TEST]: association loss handling OK\n");
  return EXIT_SUCCESS;
}
