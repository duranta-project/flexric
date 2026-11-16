/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include "seq_report_sty.h"

#include <assert.h>

void free_seq_report_sty_dapp_sm(seq_report_sty_dapp_sm_t* src)
{
  assert(src != NULL);
  assert(src->name.len > 0 && src->name.len < 151);
  free_byte_array(src->name);

  if (src->dapp_e3_subs != NULL) {
    free_dapp_e3_subscription_list(src->dapp_e3_subs);
    free(src->dapp_e3_subs);
  }
}

seq_report_sty_dapp_sm_t cp_seq_report_sty_dapp_sm(seq_report_sty_dapp_sm_t const* src)
{
  assert(src != NULL);
  seq_report_sty_dapp_sm_t dst = {0};

  dst.report_type = src->report_type;

  assert(src->name.len > 0 && src->name.len < 151);
  dst.name = copy_byte_array(src->name);

  dst.ind_hdr_type = src->ind_hdr_type;

  dst.ind_msg_type = src->ind_msg_type;

  if (src->dapp_e3_subs != NULL) {
    dst.dapp_e3_subs = calloc(1, sizeof(dapp_e3_subscription_list_t));
    assert(dst.dapp_e3_subs != NULL && "Memory exhausted");
    *dst.dapp_e3_subs = cp_dapp_e3_subscription_list(src->dapp_e3_subs);
  }

  return dst;
}

bool eq_seq_report_sty_dapp_sm(seq_report_sty_dapp_sm_t const* m0, seq_report_sty_dapp_sm_t const* m1)
{
  if (m0 == m1)
    return true;

  if (m0 == NULL || m1 == NULL)
    return false;

  if (m0->report_type != m1->report_type)
    return false;

  assert(m0->name.len > 0 && m0->name.len < 151);
  assert(m1->name.len > 0 && m1->name.len < 151);
  if (eq_byte_array(&m0->name, &m1->name) == false)
    return false;

  if (m0->ind_hdr_type != m1->ind_hdr_type)
    return false;

  if (m0->ind_msg_type != m1->ind_msg_type)
    return false;

  if (m0->dapp_e3_subs == NULL && m1->dapp_e3_subs == NULL)
    return true;

  if (m0->dapp_e3_subs == NULL || m1->dapp_e3_subs == NULL)
    return false;

  return eq_dapp_e3_subscription_list(m0->dapp_e3_subs, m1->dapp_e3_subs);
}
