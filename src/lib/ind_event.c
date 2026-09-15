/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */


#include "ind_event.h"

#include <assert.h>
#include <stdlib.h>

int cmp_ind_event(void const* m0_v, void const* m1_v)
{
  assert(m0_v != NULL);
  assert(m1_v != NULL);
  ind_event_t* m0 = (ind_event_t*)m0_v;
  ind_event_t* m1 = (ind_event_t*)m1_v;

  int cmp = cmp_ric_gen_id(&m0->ric_id, &m1->ric_id);
  if(cmp != 0)
    return cmp;

  // A RIC ID alone is not a key: assoc_rb_tree_insert does not reject equality,
  // so two subscriptions can share one. Break the tie on the fd, unique per
  // subscription, to make this a total order and every extraction by key exact.
  // Same inverted convention as cmp_ric_gen_id above: 1 means "less than".
  if(m0->fd < m1->fd) return 1;
  if(m0->fd > m1->fd) return -1;

  return 0;
}

bool eq_ind_event(const void* value, const void* key)
{
  assert(value != NULL);
  assert(key != NULL);
  
  ric_gen_id_t* ric_id = (ric_gen_id_t*)value; 
  ind_event_t* ind_ev = (ind_event_t*)key;
  bool eq = eq_ric_gen_id(ric_id, &ind_ev->ric_id );
  return eq;
}


/*
void free_ind_event(ind_event_t* src)
{
  assert(src != NULL);

//  ric_gen_id_t ric_id;
  // Non-owning ptr
//  sm_agent_t* sm;
//  uint8_t action_id;
  free_subscribe_timer(&src->sub);
}
*/

