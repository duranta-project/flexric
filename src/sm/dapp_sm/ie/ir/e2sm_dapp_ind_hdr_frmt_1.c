/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include "e2sm_dapp_ind_hdr_frmt_1.h"
#include <string.h>

void free_e2sm_dapp_ind_hdr_frmt_1(e2sm_dapp_ind_hdr_frmt_1_t* src)
{
  assert(src != NULL);
  // All fields are value types — nothing heap-allocated to free.
}

bool eq_e2sm_dapp_ind_hdr_frmt_1(e2sm_dapp_ind_hdr_frmt_1_t const* m0, e2sm_dapp_ind_hdr_frmt_1_t const* m1)
{
  if (m0 == m1)
    return true;
  if (m0 == NULL || m1 == NULL)
    return false;

  if (m0->ran_function_id != m1->ran_function_id)
    return false;
  if (m0->dapp_id != m1->dapp_id)
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

e2sm_dapp_ind_hdr_frmt_1_t cp_e2sm_dapp_ind_hdr_frmt_1(e2sm_dapp_ind_hdr_frmt_1_t const* src)
{
  assert(src != NULL);

  e2sm_dapp_ind_hdr_frmt_1_t dst = {
      .ran_function_id = src->ran_function_id,
      .dapp_id = src->dapp_id,
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
