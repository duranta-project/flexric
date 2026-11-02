/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include "seq_ctrl_style.h"

#include <assert.h>
#include <string.h>

void free_seq_ctrl_style_dapp_sm(seq_ctrl_style_dapp_sm_t* src)
{
  assert(src != NULL);

  if (src->name.buf != NULL) {
    free(src->name.buf);
    src->name.buf = NULL;
    src->name.len = 0;
  }

  if (src->dapp_e3_subs != NULL) {
    free_dapp_e3_subscription_list(src->dapp_e3_subs);
    free(src->dapp_e3_subs);
  }
}

bool eq_seq_ctrl_style_dapp_sm(seq_ctrl_style_dapp_sm_t const* m0, seq_ctrl_style_dapp_sm_t const* m1)
{
  if (m0 == m1)
    return true;

  if (m0 == NULL || m1 == NULL)
    return false;

  if (m0->style_type != m1->style_type)
    return false;

  if (m0->name.len != m1->name.len)
    return false;

  if (m0->name.len > 0) {
    assert(m0->name.buf != NULL);
    assert(m1->name.buf != NULL);
    if (memcmp(m0->name.buf, m1->name.buf, m0->name.len) != 0)
      return false;
  }

  if (m0->hdr != m1->hdr)
    return false;

  if (m0->msg != m1->msg)
    return false;

  if (m0->out_frmt != m1->out_frmt)
    return false;

  if (m0->dapp_e3_subs == NULL && m1->dapp_e3_subs == NULL)
    return true;

  if (m0->dapp_e3_subs == NULL || m1->dapp_e3_subs == NULL)
    return false;

  return eq_dapp_e3_subscription_list(m0->dapp_e3_subs, m1->dapp_e3_subs);
}

seq_ctrl_style_dapp_sm_t cp_seq_ctrl_style_dapp_sm(seq_ctrl_style_dapp_sm_t const* src)
{
  assert(src != NULL);

  seq_ctrl_style_dapp_sm_t dst = {0};

  dst.style_type = src->style_type;

  dst.name.len = src->name.len;
  if (src->name.len > 0) {
    assert(src->name.buf != NULL);
    dst.name.buf = malloc(src->name.len);
    assert(dst.name.buf != NULL && "Memory exhausted while copying name");
    memcpy(dst.name.buf, src->name.buf, src->name.len);
  } else {
    dst.name.buf = NULL;
  }

  dst.hdr = src->hdr;

  dst.msg = src->msg;

  dst.out_frmt = src->out_frmt;

  if (src->dapp_e3_subs != NULL) {
    dst.dapp_e3_subs = calloc(1, sizeof(dapp_e3_subscription_list_t));
    assert(dst.dapp_e3_subs != NULL && "Memory exhausted");
    *dst.dapp_e3_subs = cp_dapp_e3_subscription_list(src->dapp_e3_subs);
  }

  return dst;
}
