/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

/*
 * An aperiodic indication was routed by its RIC Requestor ID alone. E2AP
 * identifies a subscription by {RequestorID, InstanceID}, so the agent can hold
 * several subscriptions under one Requestor ID, and the O-RAN SC near-RT RIC
 * sends every subscription under the same one. The first entry in the tree won,
 * the highest RAN function ID, and the service model behind it was handed the
 * indication data of another one.
 */

#undef NDEBUG

#include "agent/e2_agent.h"
#include "lib/aind_event.h"
#include "lib/ind_event.h"
#include "util/compare.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static
void free_nop(void* key, void* value)
{
  (void)key;
  (void)value;
}

static
void add_aperiodic_subscription(e2_agent_t* ag, int fd, ric_gen_id_t ric_id, sm_agent_t* sm)
{
  ind_event_t ev = {.ric_id = ric_id, .sm = sm, .action_id = 1, .type = APERIODIC_SUBSCRIPTION_FLRC};
  bi_map_insert(&ag->ind_event, &fd, sizeof(fd), &ev, sizeof(ev));
}

static
void expect_routed_to(e2_agent_t* ag, ric_gen_id_t ric_id, sm_agent_t* sm, void* ind_data)
{
  e2_async_event_agent(ag, ric_id, ind_data);

  assert(size_tsq(&ag->aind) == 1);
  aind_event_t aind = {0};
  pop_tsq(&ag->aind, &aind, sizeof(aind));

  assert(aind.sm == sm);
  assert(aind.ind_data == ind_data);
  assert(aind.ric_id.ran_func_id == ric_id.ran_func_id);
  assert(aind.ric_id.ric_req_id == ric_id.ric_req_id);
  assert(aind.ric_id.ric_inst_id == ric_id.ric_inst_id);
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

  // Same Requestor ID, three subscriptions: two RAN functions, and one of them twice
  uint32_t const req_id = 123;
  ric_gen_id_t const rc_id = {.ric_req_id = req_id, .ric_inst_id = 0, .ran_func_id = 3};
  ric_gen_id_t const mac_id = {.ric_req_id = req_id, .ric_inst_id = 0, .ran_func_id = 142};
  ric_gen_id_t const mac_id_inst1 = {.ric_req_id = req_id, .ric_inst_id = 1, .ran_func_id = 142};

  sm_agent_t sm_rc = {0};
  sm_agent_t sm_mac = {0};
  sm_agent_t sm_mac_inst1 = {0};

  // Distinct, negative fds as the agent hands them out
  add_aperiodic_subscription(ag, -1, rc_id, &sm_rc);
  add_aperiodic_subscription(ag, -2, mac_id, &sm_mac);
  add_aperiodic_subscription(ag, -3, mac_id_inst1, &sm_mac_inst1);

  int data = 0;
  expect_routed_to(ag, rc_id, &sm_rc, &data);
  expect_routed_to(ag, mac_id, &sm_mac, &data);
  expect_routed_to(ag, mac_id_inst1, &sm_mac_inst1, &data);

  printf("[TEST]: aperiodic indications reach the subscription with the whole RIC ID\n");
  return 0;
}
