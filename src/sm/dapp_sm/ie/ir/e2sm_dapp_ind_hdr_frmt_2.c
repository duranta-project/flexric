/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include "e2sm_dapp_ind_hdr_frmt_2.h"
#include <string.h>

void free_e2sm_dapp_ind_hdr_frmt_2(e2sm_dapp_ind_hdr_frmt_2_t* src)
{
  assert(src != NULL);
  // All fields are value types — nothing heap-allocated to free.
}

bool eq_e2sm_dapp_ind_hdr_frmt_2(e2sm_dapp_ind_hdr_frmt_2_t const* m0, e2sm_dapp_ind_hdr_frmt_2_t const* m1)
{
  if (m0 == m1)
    return true;
  if (m0 == NULL || m1 == NULL)
    return false;

  if (m0->node_type != m1->node_type)
    return false;
  if (memcmp(m0->node_plmn_id, m1->node_plmn_id, 3) != 0)
    return false;
  if (m0->node_nb_id != m1->node_nb_id)
    return false;

  if (m0->node_cu_du_id_present != m1->node_cu_du_id_present)
    return false;
  if (m0->node_cu_du_id_present && m0->node_cu_du_id != m1->node_cu_du_id)
    return false;

  if (m0->timestamp_ns != m1->timestamp_ns)
    return false;
  if (m0->sequence_id != m1->sequence_id)
    return false;

  return true;
}

e2sm_dapp_ind_hdr_frmt_2_t cp_e2sm_dapp_ind_hdr_frmt_2(e2sm_dapp_ind_hdr_frmt_2_t const* src)
{
  assert(src != NULL);

  e2sm_dapp_ind_hdr_frmt_2_t dst = {
      .node_type = src->node_type,
      .node_nb_id = src->node_nb_id,
      .node_cu_du_id_present = src->node_cu_du_id_present,
      .node_cu_du_id = src->node_cu_du_id,
      .timestamp_ns = src->timestamp_ns,
      .sequence_id = src->sequence_id,
  };
  memcpy(dst.node_plmn_id, src->node_plmn_id, 3);

  return dst;
}