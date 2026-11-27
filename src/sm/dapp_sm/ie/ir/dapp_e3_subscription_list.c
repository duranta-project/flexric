/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include "dapp_e3_subscription_list.h"

#include <assert.h>

void free_dapp_e3_subscription_list(dapp_e3_subscription_list_t* src)
{
  assert(src != NULL);
  assert(src->sz_dapp_e3_subscriptions > 0 && src->sz_dapp_e3_subscriptions < 256);
  for (size_t i = 0; i < src->sz_dapp_e3_subscriptions; ++i) {
    free_dapp_e3_subscription_item(&src->dapp_e3_subscriptions[i]);
  }
  free(src->dapp_e3_subscriptions);
  src->dapp_e3_subscriptions = NULL;
  src->sz_dapp_e3_subscriptions = 0;
}

bool eq_dapp_e3_subscription_list(dapp_e3_subscription_list_t const* m0, dapp_e3_subscription_list_t const* m1)
{
  if (m0 == m1)
    return true;

  if (m0 == NULL || m1 == NULL)
    return false;

  if (m0->sz_dapp_e3_subscriptions != m1->sz_dapp_e3_subscriptions)
    return false;

  for (size_t i = 0; i < m0->sz_dapp_e3_subscriptions; ++i) {
    if (eq_dapp_e3_subscription_item(&m0->dapp_e3_subscriptions[i], &m1->dapp_e3_subscriptions[i]) == false)
      return false;
  }

  return true;
}

dapp_e3_subscription_list_t cp_dapp_e3_subscription_list(dapp_e3_subscription_list_t const* src)
{
  assert(src != NULL);
  dapp_e3_subscription_list_t dst = {0};
  assert(src->sz_dapp_e3_subscriptions > 0 && src->sz_dapp_e3_subscriptions < 256);

  dst.sz_dapp_e3_subscriptions = src->sz_dapp_e3_subscriptions;
  dst.dapp_e3_subscriptions = calloc(dst.sz_dapp_e3_subscriptions, sizeof(dapp_e3_subscription_item_t));
  assert(dst.dapp_e3_subscriptions != NULL && "Memory exhausted");

  for (size_t i = 0; i < src->sz_dapp_e3_subscriptions; ++i) {
    dst.dapp_e3_subscriptions[i] = cp_dapp_e3_subscription_item(&src->dapp_e3_subscriptions[i]);
  }

  return dst;
}