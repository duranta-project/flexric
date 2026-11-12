/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */



#ifndef RIC_SERVICE_UPDATE_H
#define  RIC_SERVICE_UPDATE_H

#include "common/e2ap_ran_function.h"
#include "common/e2ap_ran_function_id_rev.h"
#include "common/e2ap_global_node_id.h"

typedef struct {
  uint8_t trans_id;

  ran_function_t* added;
  size_t len_added;

  ran_function_t* modified;
  size_t len_modified;

  e2ap_ran_function_id_rev_t* deleted;
  size_t len_deleted;

  global_e2_node_id_t e2_node_id;
} ric_service_update_t;

bool eq_ric_service_update(const ric_service_update_t* m0, const ric_service_update_t* m1);

#endif
