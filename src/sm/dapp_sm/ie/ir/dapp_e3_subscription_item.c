/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include "dapp_e3_subscription_item.h"

#include <assert.h>
#include <string.h>

void free_dapp_e3_subscription_item(dapp_e3_subscription_item_t* src)
{
  assert(src != NULL);
  free(src->subscribed_e3_ran_functions);
  src->subscribed_e3_ran_functions = NULL;
  src->sz_subscribed_e3_ran_functions = 0;
}

bool eq_dapp_e3_subscription_item(dapp_e3_subscription_item_t const* m0, dapp_e3_subscription_item_t const* m1)
{
  if (m0 == m1)
    return true;

  if (m0 == NULL || m1 == NULL)
    return false;

  if (m0->dapp_id != m1->dapp_id)
    return false;

  if (m0->sz_subscribed_e3_ran_functions != m1->sz_subscribed_e3_ran_functions)
    return false;

  for (size_t i = 0; i < m0->sz_subscribed_e3_ran_functions; ++i) {
    if (m0->subscribed_e3_ran_functions[i] != m1->subscribed_e3_ran_functions[i])
      return false;
  }

  return true;
}

dapp_e3_subscription_item_t cp_dapp_e3_subscription_item(dapp_e3_subscription_item_t const* src)
{
  assert(src != NULL);
  dapp_e3_subscription_item_t dst = {0};

  dst.dapp_id = src->dapp_id;
  dst.sz_subscribed_e3_ran_functions = src->sz_subscribed_e3_ran_functions;

  if (src->sz_subscribed_e3_ran_functions > 0) {
    assert(src->sz_subscribed_e3_ran_functions < 64);
    size_t sz = src->sz_subscribed_e3_ran_functions * sizeof(uint32_t);
    dst.subscribed_e3_ran_functions = malloc(sz);
    assert(dst.subscribed_e3_ran_functions != NULL && "Memory exhausted");
    memcpy(dst.subscribed_e3_ran_functions, src->subscribed_e3_ran_functions, sz);
  }

  return dst;
}