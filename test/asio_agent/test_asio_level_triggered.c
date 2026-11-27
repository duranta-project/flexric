/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

/*
 * The E2 agent read one message per wakeup from an edge-triggered socket.
 * When two messages arrive together the second stays in the socket buffer, and
 * with no new arrival to raise another edge the agent is never woken for it:
 * each message is only handled when a later one comes in.
 *
 * A datagram socket pair stands in for the SCTP socket, as both keep message
 * boundaries and the agent reads one message per call.
 */

#undef NDEBUG

#include "agent/asio_agent.h"

#include <assert.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

int main()
{
  asio_agent_t io;
  init_asio_agent(&io);

  int sv[2];
  int rc = socketpair(AF_UNIX, SOCK_DGRAM, 0, sv);
  assert(rc == 0);

  add_fd_level_asio_agent(&io, sv[0]);

  // Two messages arrive together
  char const msg[2] = {'a', 'b'};
  assert(write(sv[1], &msg[0], 1) == 1);
  assert(write(sv[1], &msg[1], 1) == 1);

  // The agent reads one message per wakeup
  char c = 0;
  assert(event_asio_agent(&io) == sv[0]);
  assert(read(sv[0], &c, 1) == 1 && c == 'a');

  // The second one is still unread, so the agent must be woken again
  assert(event_asio_agent(&io) == sv[0]);
  assert(read(sv[0], &c, 1) == 1 && c == 'b');

  // Nothing left, no spurious wakeup
  assert(event_asio_agent(&io) == -1);

  close(sv[0]);
  close(sv[1]);

  printf("[TEST]: a message left in the E2 socket wakes the agent again\n");
  return 0;
}
