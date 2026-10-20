/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

/*
 * The agent notifies its epoll loop of every queued aperiodic indication with
 * a token in a pipe. e2_async_event_agent() wrote the decimal string of the
 * RIC Requestor ID as the token: one byte for id 2, two for 21, four for 1234.
 * The loop reads one item per token and asserts that a read never returns a
 * single byte, so a Requestor ID below 10 aborted the RAN process.
 *
 * One queued indication must put exactly one byte in the pipe, whatever the
 * Requestor ID.
 */

#undef NDEBUG

#include "agent/e2_agent.h"
#include "lib/aind_event.h"
#include "lib/ind_event.h"
#include "util/compare.h"

#include <assert.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static
void free_nop(void* key, void* value)
{
  (void)key;
  (void)value;
}

static
void add_aperiodic_subscription(e2_agent_t* ag, int fd, uint32_t ric_req_id, sm_agent_t* sm)
{
  ric_gen_id_t const ric_id = {.ric_req_id = ric_req_id, .ric_inst_id = 0, .ran_func_id = 3};
  ind_event_t ev = {.ric_id = ric_id, .sm = sm, .action_id = 1, .type = APERIODIC_SUBSCRIPTION_FLRC};
  bi_map_insert(&ag->ind_event, &fd, sizeof(fd), &ev, sizeof(ev));
}

// Bytes sitting in the pipe, without blocking
static
ssize_t drain_pipe(int fd)
{
  char buf[64];
  ssize_t total = 0;
  for(;;){
    ssize_t const n = read(fd, buf, sizeof(buf));
    if(n <= 0)
      break;
    total += n;
  }
  return total;
}

int main()
{
  e2_agent_t* ag = calloc(1, sizeof(*ag));
  assert(ag != NULL);

  int rc = pthread_mutex_init(&ag->mtx_ind_event, NULL);
  assert(rc == 0);
  init_asio_agent(&ag->io);
  init_tsq(&ag->aind, sizeof(aind_event_t));
  bi_map_init(&ag->ind_event, sizeof(int), sizeof(ind_event_t), cmp_fd, cmp_ind_event, free_nop, free_nop);

  // The agent's read end is blocking, the test must not block on an empty pipe
  int const flags = fcntl(ag->io.pipe.r, F_GETFL, 0);
  assert(flags != -1);
  rc = fcntl(ag->io.pipe.r, F_SETFL, flags | O_NONBLOCK);
  assert(rc != -1);

  // One, two and four digits
  uint32_t const ids[] = {2, 21, 1234};
  sm_agent_t sm = {0};
  int data = 0;

  for(size_t i = 0; i < sizeof(ids) / sizeof(ids[0]); ++i){
    add_aperiodic_subscription(ag, -1 - (int)i, ids[i], &sm);

    e2_async_event_agent(ag, ids[i], &data);

    assert(drain_pipe(ag->io.pipe.r) == 1);
  }

  assert(size_tsq(&ag->aind) == 3);

  printf("[TEST]: one aperiodic indication puts one token in the pipe\n");
  return 0;
}
