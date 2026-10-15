/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

/*
 * A signal delivered while the E2 agent waits in epoll_wait() used to abort
 * the RAN process: event_asio_agent() asserted on any epoll_wait() error,
 * EINTR included. Attaching a debugger to a running gNB is enough to trigger it.
 */

#undef NDEBUG

#include "agent/asio_agent.h"

#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

static
void on_signal(int sig)
{
  (void)sig;
}

static
long elapsed_ms(struct timespec const* t0)
{
  struct timespec t1;
  clock_gettime(CLOCK_MONOTONIC, &t1);
  return (t1.tv_sec - t0->tv_sec) * 1000 + (t1.tv_nsec - t0->tv_nsec) / 1000000;
}

int main()
{
  asio_agent_t io;
  init_asio_agent(&io);

  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = on_signal;
  sigemptyset(&sa.sa_mask);
  int rc = sigaction(SIGALRM, &sa, NULL);
  assert(rc == 0);

  // event_asio_agent() waits 1000 ms at most, the signal lands well before
  struct itimerval const it = {.it_value = {.tv_sec = 0, .tv_usec = 100 * 1000}};
  rc = setitimer(ITIMER_REAL, &it, NULL);
  assert(rc == 0);

  struct timespec t0;
  clock_gettime(CLOCK_MONOTONIC, &t0);

  // Nothing is registered but the internal pipe: an interrupted wait must
  // return the same "nothing ready" as a timeout does
  int const fd = event_asio_agent(&io);
  assert(fd == -1);

  // returned because of the signal, not because the wait timed out
  assert(elapsed_ms(&t0) < 900);

  printf("[TEST]: epoll_wait interrupted by a signal does not abort the agent\n");
  return 0;
}
