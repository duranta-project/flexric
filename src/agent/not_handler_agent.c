/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include "not_handler_agent.h"
#include "msg_handler_agent.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include "../lib/pending_events.h"

// Why a notification ends the E2 session with the nearRT-RIC, or NULL when it
// does not. The association can end three ways:
//  - SCTP_SHUTDOWN_EVENT: the RIC closed it gracefully.
//  - SCTP_COMM_LOST: it died (RIC host or network gone, or the peer sent an
//    ABORT); no SHUTDOWN is sent.
//  - SCTP_RESTART: the RIC came back under the same addresses and re-joined the
//    association, having forgotten every subscription and the E2 setup.
// The other association states are not an end: SCTP_COMM_UP is one coming up,
// SCTP_CANT_STR_ASSOC a setup attempt the retry timer repeats, and
// SCTP_SHUTDOWN_COMP closes the shutdown already reported.
static
char const* e2_session_end_reason(union sctp_notification const* notif)
{
  assert(notif != NULL);

  if(notif->sn_header.sn_type == SCTP_SHUTDOWN_EVENT)
    return "the nearRT-RIC shut the association down";

  if(notif->sn_header.sn_type != SCTP_ASSOC_CHANGE)
    return NULL;

  switch(notif->sn_assoc_change.sac_state){
    case SCTP_COMM_LOST:
      return "the association with the nearRT-RIC was lost";
    case SCTP_RESTART:
      return "the nearRT-RIC restarted";
    default:
      return NULL;
  }
}

// The E2 session is over: drop what it installed and start a new one.
static
void end_e2_session(e2_agent_t* ag)
{
  assert(ag != NULL);

  // First, so that nothing queued behind this goes out on the association the
  // next E2 SETUP REQUEST opens before the RIC has answered it.
  ag->e2_session_up = false;

  stop_all_ind_events(ag);

  // A pending event is created along with a timer of 3000 ms,
  // after which an event will be generated.
  //
  // Only ever one: pend_event() asserts the map holds exactly one entry, so a
  // second notification arriving before the setup response completes would abort
  // the RAN on the next timer tick.
  if(bi_map_size(&ag->pending) == 0){
    pending_event_t ev = SETUP_REQUEST_PENDING_EVENT;
    long const wait_ms = 3000;
    int fd_timer = create_timer_ms_asio_agent(&ag->io, wait_ms, wait_ms);

    bi_map_insert(&ag->pending, &fd_timer, sizeof(fd_timer), &ev, sizeof(ev));
  }
}

void notification_handle_ag(e2_agent_t* ag, sctp_msg_t const* msg)
{
  assert(ag != NULL);
  assert(msg != NULL && msg->type == SCTP_MSG_NOTIFICATION);

  char const* reason = e2_session_end_reason(msg->notif);
  if(reason == NULL){
    printf("[E2 AGENT]: SCTP notification type %u ignored\n", msg->notif->sn_header.sn_type);
    return;
  }

  printf("[E2 AGENT]: E2 session with the nearRT-RIC ended: %s\n", reason);

  end_e2_session(ag);
}
