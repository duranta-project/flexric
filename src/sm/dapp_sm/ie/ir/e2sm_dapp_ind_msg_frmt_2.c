/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include "e2sm_dapp_ind_msg_frmt_2.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

void free_e2sm_dapp_ind_msg_frmt_2(e2sm_dapp_ind_msg_frmt_2_t* src)
{
  assert(src != NULL);

  for (size_t i = 0; i < src->dapp_e3_subs.sz_dapp_e3_subscriptions; i++) {
    free(src->dapp_e3_subs.dapp_e3_subscriptions[i].subscribed_e3_ran_functions);
  }
  free(src->dapp_e3_subs.dapp_e3_subscriptions);
}

bool eq_e2sm_dapp_ind_msg_frmt_2(e2sm_dapp_ind_msg_frmt_2_t const* m0, e2sm_dapp_ind_msg_frmt_2_t const* m1)
{
  if (m0 == m1)
    return true;
  if (m0 == NULL || m1 == NULL)
    return false;

  if (m0->dapp_e3_subs.sz_dapp_e3_subscriptions != m1->dapp_e3_subs.sz_dapp_e3_subscriptions)
    return false;

  for (size_t i = 0; i < m0->dapp_e3_subs.sz_dapp_e3_subscriptions; i++) {
    const dapp_e3_subscription_item_t* a = &m0->dapp_e3_subs.dapp_e3_subscriptions[i];
    const dapp_e3_subscription_item_t* b = &m1->dapp_e3_subs.dapp_e3_subscriptions[i];

    if (a->dapp_id != b->dapp_id)
      return false;

    if (a->sz_subscribed_e3_ran_functions != b->sz_subscribed_e3_ran_functions)
      return false;

    for (size_t j = 0; j < a->sz_subscribed_e3_ran_functions; j++) {
      if (a->subscribed_e3_ran_functions[j] != b->subscribed_e3_ran_functions[j])
        return false;
    }
  }

  return true;
}

e2sm_dapp_ind_msg_frmt_2_t cp_e2sm_dapp_ind_msg_frmt_2(e2sm_dapp_ind_msg_frmt_2_t const* src)
{
  assert(src != NULL);

  e2sm_dapp_ind_msg_frmt_2_t dst = {0};

  dst.dapp_e3_subs.sz_dapp_e3_subscriptions = src->dapp_e3_subs.sz_dapp_e3_subscriptions;

  if (src->dapp_e3_subs.sz_dapp_e3_subscriptions > 0) {
    size_t n = src->dapp_e3_subs.sz_dapp_e3_subscriptions;
    dst.dapp_e3_subs.dapp_e3_subscriptions = calloc(n, sizeof(dapp_e3_subscription_item_t));
    assert(dst.dapp_e3_subs.dapp_e3_subscriptions != NULL && "Memory exhausted");

    for (size_t i = 0; i < n; i++) {
      const dapp_e3_subscription_item_t* s = &src->dapp_e3_subs.dapp_e3_subscriptions[i];
      dapp_e3_subscription_item_t* d = &dst.dapp_e3_subs.dapp_e3_subscriptions[i];

      d->dapp_id = s->dapp_id;
      d->sz_subscribed_e3_ran_functions = s->sz_subscribed_e3_ran_functions;

      if (s->sz_subscribed_e3_ran_functions > 0) {
        size_t sz = s->sz_subscribed_e3_ran_functions * sizeof(uint32_t);
        d->subscribed_e3_ran_functions = malloc(sz);
        assert(d->subscribed_e3_ran_functions != NULL && "Memory exhausted");
        memcpy(d->subscribed_e3_ran_functions, s->subscribed_e3_ran_functions, sz);
      }
    }
  }

  return dst;
}
