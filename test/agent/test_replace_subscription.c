/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

/*
 * A RIC_SUBSCRIPTION_REQUEST for a RIC ID the agent already holds must replace
 * that subscription, not add a second one under the same key.
 *
 * The subscription registry is a multiset (assoc_rb_tree_insert does not reject
 * equality), so a RIC that re-installs a subscription whose delete never arrived,
 * or that reuses one Requestor ID as the O-RAN SC near-RT RIC does, left two
 * entries, each with its own action definition and timer, behind one RIC ID.
 */

// The asserts are the test: keep them in builds that define NDEBUG.
#undef NDEBUG

#include "agent/e2_agent.h"
#include "agent/asio_agent.h"
#include "agent/msg_handler_agent.h"
#include "lib/ind_event.h"
#include "util/compare.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RAN_FUNC_ID 3

static int subscribed = 0;
static int released = 0;

// What the RAN registers as free_aper_subs: un-registers the subscription.
static
void count_release(ric_gen_id_t ric_id)
{
  (void)ric_id;
  ++released;
}

static
sm_ag_if_ans_subs_t on_subscription_stub(sm_agent_t const* sm, sm_subs_data_t const* data)
{
  (void)sm;
  (void)data;
  ++subscribed;
  sm_ag_if_ans_subs_t ans = {.type = APERIODIC_SUBSCRIPTION_FLRC};
  ans.aper.free_aper_subs = count_release;
  return ans;
}

static
uint16_t id_stub(void)
{
  return RAN_FUNC_ID;
}

static
int cmp_ran_func_id_test(const void* a, const void* b)
{
  uint16_t const x = *(uint16_t const*)a;
  uint16_t const y = *(uint16_t const*)b;
  return x < y ? 1 : (x > y ? -1 : 0);
}

static
void free_nop(void* key, void* value)
{
  (void)key;
  (void)value;
}

static
e2ap_msg_t subscription_request(ric_gen_id_t ric_id)
{
  static ric_action_t action = {0};
  action.id = 1;
  action.type = RIC_ACT_REPORT;

  e2ap_msg_t msg = {.type = RIC_SUBSCRIPTION_REQUEST};
  msg.u_msgs.ric_sub_req.ric_id = ric_id;
  msg.u_msgs.ric_sub_req.action = &action;
  msg.u_msgs.ric_sub_req.len_action = 1;
  return msg;
}

int main()
{
  e2_agent_t* ag = calloc(1, sizeof(*ag));
  assert(ag != NULL);

  init_asio_agent(&ag->io);
  int rc = pthread_mutex_init(&ag->mtx_ind_event, NULL);
  assert(rc == 0);
  bi_map_init(&ag->ind_event, sizeof(int), sizeof(ind_event_t), cmp_fd, cmp_ind_event, free_nop, free_nop);

  // A service model registry holding a single stub, as the agent's plugin loader would
  sm_agent_t sm = {0};
  sm.proc.on_subscription = on_subscription_stub;
  sm.info.id = id_stub;

  rc = pthread_mutex_init(&ag->plugin.sm_ds_mtx, NULL);
  assert(rc == 0);
  assoc_init(&ag->plugin.sm_ds, sizeof(uint16_t), cmp_ran_func_id_test, free_nop);
  uint16_t const key = RAN_FUNC_ID;
  assoc_insert(&ag->plugin.sm_ds, &key, sizeof(key), &sm);

  ric_gen_id_t const ric_id = {.ric_req_id = 123, .ric_inst_id = 0, .ran_func_id = RAN_FUNC_ID};
  e2ap_msg_t const msg = subscription_request(ric_id);

  e2ap_handle_subscription_request_agent(ag, &msg);
  assert(subscribed == 1);
  assert(bi_map_size(&ag->ind_event) == 1);
  assert(released == 0);

  // The same RIC ID again: the first subscription is released, not kept beside it
  e2ap_handle_subscription_request_agent(ag, &msg);
  assert(subscribed == 2);
  assert(bi_map_size(&ag->ind_event) == 1 && "the subscription was replaced, not duplicated");
  assert(released == 1 && "the replaced subscription was released");

  // A different RIC ID is a new subscription
  ric_gen_id_t const other = {.ric_req_id = 124, .ric_inst_id = 0, .ran_func_id = RAN_FUNC_ID};
  e2ap_msg_t const msg_other = subscription_request(other);
  e2ap_handle_subscription_request_agent(ag, &msg_other);
  assert(bi_map_size(&ag->ind_event) == 2);
  assert(released == 1);

  printf("[TEST]: a subscription for an installed RIC ID replaces it\n");
  return 0;
}
