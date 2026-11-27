/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef DAPP_E3_SUBSCRIPTION_LIST_H
#define DAPP_E3_SUBSCRIPTION_LIST_H

#include "dapp_e3_subscription_item.h"

#include <stdbool.h>
#include <stdlib.h>

typedef struct {
  size_t sz_dapp_e3_subscriptions;
  dapp_e3_subscription_item_t* dapp_e3_subscriptions;
} dapp_e3_subscription_list_t;

void free_dapp_e3_subscription_list(dapp_e3_subscription_list_t* src);

bool eq_dapp_e3_subscription_list(dapp_e3_subscription_list_t const* m0, dapp_e3_subscription_list_t const* m1);

dapp_e3_subscription_list_t cp_dapp_e3_subscription_list(dapp_e3_subscription_list_t const* src);

#endif