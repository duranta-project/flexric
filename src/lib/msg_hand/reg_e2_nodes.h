/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef REGISTERED_E2_NODES_H
#define REGISTERED_E2_NODES_H 

#include "../../ric/plugin_ric.h"
#include "../../util/alg_ds/ds/assoc_container/assoc_generic.h"
#include "../e2ap/e2_node_connected_wrapper.h"
#include "e2_node_arr.h"
#include "../../xApp/e2_node_arr_xapp.h"
#ifdef E2AP_V1
#include "../e2ap/v1_01/e2ap_types/common/e2ap_ran_function_id_rev.h"
#elif defined(E2AP_V2)
#include "../e2ap/v2_03/e2ap_types/common/e2ap_ran_function_id_rev.h"
#elif defined(E2AP_V3)
#include "../e2ap/v3_01/e2ap_types/common/e2ap_ran_function_id_rev.h"
#else
static_assert(0!=0, "Unknown E2AP Version");
#endif

#include <stddef.h>
#include <stdint.h>
#include <pthread.h>

typedef struct{
  seq_arr_t ran_func; //  ran_function_t
#ifdef E2AP_V1  
#elif defined (E2AP_V2) || defined(E2AP_V3)
  seq_arr_t comp_conf_add; // e2ap_node_component_config_add_t
#endif         
} pair_rf_cca_t;

void free_pair_rf_cca(pair_rf_cca_t* src);  

typedef struct{
  // key:global_e2_node_id_t | value: pair_rf_cca_t* of seq_arr_t of ran_function_t and 
  assoc_rb_tree_t node_to_rf;  
  pthread_mutex_t mtx;

} reg_e2_nodes_t;

void init_reg_e2_node(reg_e2_nodes_t* n); 

void free_reg_e2_node(reg_e2_nodes_t* n); 

#ifdef E2AP_V1
void add_reg_e2_node_v1(reg_e2_nodes_t* i, global_e2_node_id_t const* id, size_t len_rf, ran_function_t const* ran_func);
#else
void add_reg_e2_node(reg_e2_nodes_t* i, global_e2_node_id_t const* id, size_t len_rf, ran_function_t const* ran_func, size_t len_cca, e2ap_node_component_config_add_t const* cca);
#endif

void rm_reg_e2_node(reg_e2_nodes_t* n, global_e2_node_id_t const* id);

size_t sz_reg_e2_node(reg_e2_nodes_t* n);

assoc_rb_tree_t cp_reg_e2_node(reg_e2_nodes_t* n); 

e2_node_arr_t generate_e2_node_arr(reg_e2_nodes_t* n);

e2_node_arr_xapp_t generate_e2_node_arr_xapp(reg_e2_nodes_t* n, plugin_ric_t const* plg_ric);

void update_reg_e2_node(reg_e2_nodes_t* n,
                        global_e2_node_id_t const* id,
                        size_t len_added,    ran_function_t const* added,
                        size_t len_modified,  ran_function_t const* modified,
                        size_t len_deleted,   e2ap_ran_function_id_rev_t const* deleted);

#endif

