/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include "e2sm_dapp_ind_msg_frmt_1.h"

#include <assert.h>
#include <string.h>

void free_e2sm_dapp_ind_msg_frmt_1(e2sm_dapp_ind_msg_frmt_1_t* src)
{
  if (!src)
    return;

  if (src->data) {
    free(src->data);
    src->data = NULL;
  }
  src->data_size = 0;
}

bool eq_e2sm_dapp_ind_msg_frmt_1(e2sm_dapp_ind_msg_frmt_1_t const* m0, e2sm_dapp_ind_msg_frmt_1_t const* m1)
{
  if (m0 == m1) {
    return true;
  }

  if (!m0 || !m1) {
    return false;
  }

  if (m0->data_size != m1->data_size) {
    return false;
  }

  if (m0->data_size == 0) {
    return true;
  }

  if (!m0->data || !m1->data) {
    return false;
  }

  return memcmp(m0->data, m1->data, m0->data_size) == 0;
}

e2sm_dapp_ind_msg_frmt_1_t cp_e2sm_dapp_ind_msg_frmt_1(e2sm_dapp_ind_msg_frmt_1_t const* src)
{
  assert(src);

  e2sm_dapp_ind_msg_frmt_1_t dst = {.data_size = src->data_size, .data = NULL};

  if (src->data_size > 0) {
    dst.data = malloc(src->data_size);
    assert(dst.data);
    memcpy(dst.data, src->data, src->data_size);
  }

  return dst;
}
