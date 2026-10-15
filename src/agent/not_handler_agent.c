/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include "not_handler_agent.h"
#include "msg_handler_agent.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include "../lib/pending_events.h"

void notification_handle_ag(e2_agent_t* ag, sctp_msg_t const* msg)
{
  assert(ag != NULL);
  assert(msg != NULL && msg->type == SCTP_MSG_NOTIFICATION);

  // next_async_event_agent routes every SCTP notification here, and
  // e2ap_recv_sctp_msg does not filter on sn_type either. Only the shutdown
  // ends the E2 session: cp_sctp_notification decodes SCTP_COMM_UP too, and
  // dropping every subscription on an association coming up would cause an
  // outage.
  if(msg->notif->sn_header.sn_type != SCTP_SHUTDOWN_EVENT){
    printf("[E2 AGENT]: SCTP notification type %u ignored\n", msg->notif->sn_header.sn_type);
    return;
  }

  printf("[E2 AGENT]: Communication with the nearRT-RIC lost\n" );

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
