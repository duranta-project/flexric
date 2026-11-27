/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

/*
 * The agent's subscription registry is a multiset: assoc_rb_tree_insert does not
 * reject equality and cmp_ind_event compared RIC IDs only, so a RIC that installs
 * a subscription the agent already holds, or one that reuses a single RIC
 * Requestor ID as the O-RAN SC near-RT RIC does, leaves several entries under one
 * RIC ID.
 *
 * A delete found its entry with a linear scan but unlinked it with a binary
 * search on the same ambiguous key, which is not the same node. One entry's
 * action definition was freed while another was unlinked, leaving a live entry
 * with an armed timer pointing at freed memory.
 */

#undef NDEBUG

#include "agent/e2_agent.h"
#include "agent/msg_handler_agent.h"
#include "lib/ind_event.h"
#include "util/compare.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_SUBS 3

static int act_def[NUM_SUBS];
static int freed[NUM_SUBS];

static
void free_act_def_stub(sm_agent_t* sm, void* ad)
{
  (void)sm;
  for(int i = 0; i < NUM_SUBS; ++i){
    if(ad == &act_def[i]){
      // freeing one twice is the bug
      assert(freed[i] == 0);
      freed[i] += 1;
      return;
    }
  }
  assert(0 && "freeing an action definition that is not one of ours");
}

static
void free_nop(void* key, void* value)
{
  (void)key;
  (void)value;
}

int main()
{
  e2_agent_t* ag = calloc(1, sizeof(*ag));
  assert(ag != NULL);

  int rc = pthread_mutex_init(&ag->mtx_ind_event, NULL);
  assert(rc == 0);
  init_asio_agent(&ag->io);
  bi_map_init(&ag->ind_event, sizeof(int), sizeof(ind_event_t), cmp_fd, cmp_ind_event, free_nop, free_nop);

  sm_agent_t sm = {0};
  sm.free_act_def = free_act_def_stub;

  // The same RIC ID, installed three times
  ric_gen_id_t const ric_id = {.ric_req_id = 123, .ric_inst_id = 0, .ran_func_id = 3};

  for(int i = 0; i < NUM_SUBS; ++i){
    int const fd = create_timer_ms_asio_agent(&ag->io, 1000, 1000);
    ind_event_t ev = {.ric_id = ric_id, .sm = &sm, .action_id = 1,
                      .type = PERIODIC_SUBSCRIPTION_FLRC, .act_def = &act_def[i], .fd = fd};
    bi_map_insert(&ag->ind_event, &fd, sizeof(fd), &ev, sizeof(ev));
  }
  assert(assoc_rb_tree_size(&ag->ind_event.left) == NUM_SUBS);
  assert(assoc_rb_tree_size(&ag->ind_event.right) == NUM_SUBS);

  e2ap_msg_t msg = {.type = RIC_SUBSCRIPTION_DELETE_REQUEST};
  msg.u_msgs.ric_sub_del_req.ric_id = ric_id;

  for(int n = 1; n <= NUM_SUBS; ++n){
    e2ap_handle_subscription_delete_request_agent(ag, &msg);

    // One entry gone, in both trees, and its action definition freed once
    assert(assoc_rb_tree_size(&ag->ind_event.left) == (size_t)(NUM_SUBS - n));
    assert(assoc_rb_tree_size(&ag->ind_event.right) == (size_t)(NUM_SUBS - n));

    int num_freed = 0;
    for(int i = 0; i < NUM_SUBS; ++i)
      num_freed += freed[i];
    assert(num_freed == n);
  }

  printf("[TEST]: each delete removes the entry whose action definition it frees\n");
  return 0;
}
