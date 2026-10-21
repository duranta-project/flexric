/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef DAPP_SUBSCRIPTION_ITEM_H
#define DAPP_SUBSCRIPTION_ITEM_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct {
  uint32_t dapp_id;
  size_t sz_subscribed_e3_ran_functions;
  uint32_t* subscribed_e3_ran_functions;
} dapp_e3_subscription_item_t;

void free_dapp_e3_subscription_item(dapp_e3_subscription_item_t* src);

bool eq_dapp_e3_subscription_item(dapp_e3_subscription_item_t const* m0, dapp_e3_subscription_item_t const* m1);

dapp_e3_subscription_item_t cp_dapp_e3_subscription_item(dapp_e3_subscription_item_t const* src);

#endif