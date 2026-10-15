/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include "e2_agent_api.h"

#include <assert.h>                                        // for assert
#include <pthread.h>                                       // for pthread_cr...
#include <stdatomic.h>
#include <string.h>                                        // for strdup, strlen
#include <stdlib.h>
#include <stdio.h>                                         // for NULL
#include "e2_agent.h"                                      // for e2_free_agent
#include "lib/e2ap/e2ap_global_node_id_wrapper.h"  // for global_e2_...
#include "lib/e2ap/e2ap_plmn_wrapper.h"            // for plmn_t
#include "util/ngran_types.h"                              // for ngran_gNB
#include "util/conf_file.h"


static
e2_agent_t* agent = NULL;

static
pthread_t thrd_agent;

static inline
void* static_start_agent(void* a)
{
  (void)a;
  // Blocking...
  e2_start_agent(agent);
  return NULL;
}

                    
static
global_e2_node_id_t init_ge2ni(ngran_node_t ran_type, e2ap_plmn_t plmn, int nb_id, int cu_du_id)
{
  global_e2_node_id_t ge2ni =  {.type = ran_type, .plmn = plmn, .nb_id.nb_id = nb_id, .nb_id.unused = 0, .cu_du_id = NULL};

  // NODE_IS_CU is an abuse, but there is no way in the standard to differentitate
  // between NODE_IS_GNB and NODE_IS_CU. Blame the standard 
  if (NODE_IS_CU(ran_type) || NODE_IS_CUUP(ran_type) || NODE_IS_DU(ran_type)) {
    assert(cu_du_id > 0);
    ge2ni.cu_du_id = calloc(1, sizeof(uint64_t));
    assert(ge2ni.cu_du_id != NULL && "memory exhausted");
    *ge2ni.cu_du_id = cu_du_id;
  } else if(NODE_IS_MONOLITHIC(ran_type)){
  } else {
    assert(0 != 0 && "not support RAN type\n");
  }

  return ge2ni;
}



void init_agent_api(int mcc, 
                    int mnc, 
                    int mnc_digit_len,
                    int nb_id,
                    int cu_du_id,
                    ngran_node_t ran_type,
                    sm_io_ag_ran_t io,
		                fr_args_t const* args)
{
  assert(agent == NULL);  
  assert(mcc > 0);
  assert(mnc > 0);
  assert(mnc_digit_len > 0);
  assert(nb_id > 0);
  assert(ran_type >= 0);

  char* server_ip_str = get_near_ric_ip(args);

  const e2ap_plmn_t plmn = {.mcc = mcc, .mnc = mnc, .mnc_digit_len = mnc_digit_len};
  global_e2_node_id_t ge2ni = init_ge2ni(ran_type, plmn, nb_id, cu_du_id ); 

  const int e2ap_server_port = 36421;

  char* ran_type_str = get_ngran_name(ran_type);
  char str[128] = {0};
  int it = sprintf(str, "[E2 AGENT]: nearRT-RIC IP Address = %s, PORT = %d, RAN type = %s, nb_id = %d", server_ip_str, e2ap_server_port, ran_type_str, nb_id);
  assert(it > 0);
  if(ge2ni.cu_du_id != NULL){
    it = sprintf(str+it, ", cu_du_id = %ld\n", *ge2ni.cu_du_id);
    assert(it > 0);
  } else {
    it = sprintf(str+it, "\n" );
    assert(it > 0);
  }
  assert(it < 128);
  printf("%s" ,str);

  agent = e2_init_agent(server_ip_str, e2ap_server_port, ge2ni, io, args->libs_dir);

  // Spawn a new thread for the agent
  const int rc = pthread_create(&thrd_agent, NULL, static_start_agent, NULL);
  assert(rc == 0);
  free(server_ip_str);
}

void stop_agent_api(void)
{
  assert(agent != NULL);
  e2_free_agent(agent);
  int const rc = pthread_join(thrd_agent,NULL);
  assert(rc == 0);
}

void async_event_agent_api(uint32_t ric_req_id, void* ind_data)
{
  assert(agent != NULL);
  e2_async_event_agent(agent, ric_req_id, ind_data);
}

/* Re-advertise one RAN function to the RIC with a bumped revision.
 *
 * A service model whose RAN function definition changes at runtime -- because
 * what it exposes depends on state the RAN learns after E2 SETUP -- has no way
 * to tell the RIC without this: E2 SETUP happens once. The revision is
 * per-call and monotonic, which is what makes the RIC replace the stored
 * definition rather than reject it as a duplicate. */
void trigger_ric_service_update_api(uint16_t ran_func_id)
{
  if (agent == NULL) {
    printf("[E2 AGENT]: not initialised yet, skipping RIC Service Update\n");
    return;
  }

  sm_agent_t* sm = sm_plugin_ag(&agent->plugin, ran_func_id);
  if (sm == NULL) {
    printf("[E2 AGENT]: no service model for RAN function %u, cannot send RIC Service Update\n", ran_func_id);
    return;
  }

  static _Atomic uint16_t service_update_rev = 1;

  sm_e2_setup_data_t setup = sm->proc.on_e2_setup(sm);

  ric_service_update_t su = {0};
  su.trans_id = (uint8_t)(atomic_load(&service_update_rev) % 256);

  su.len_modified = 1;
  su.modified = calloc(1, sizeof(ran_function_t));
  assert(su.modified != NULL && "Memory exhausted");

  su.modified[0].id = sm->info.id();
  su.modified[0].rev = atomic_fetch_add(&service_update_rev, 1);
  su.modified[0].defn.buf = setup.ran_fun_def;
  su.modified[0].defn.len = setup.len_rfd;

  const char* oid = sm->info.oid();
  su.modified[0].oid.buf = (uint8_t*)strdup(oid);
  su.modified[0].oid.len = strlen(oid);

  e2_send_ric_service_update(agent, &su);

  free(su.modified[0].defn.buf);
  free(su.modified[0].oid.buf);
  free(su.modified);
}

void complete_control_agent_api(uint64_t key, uint8_t* ctrl_out, size_t len_out)
{
  if (agent == NULL) {
    printf("[E2 AGENT]: not initialised yet, dropping deferred control outcome (key=%lu)\n",
           (unsigned long)key);
    free(ctrl_out);
    return;
  }
  e2_agent_complete_control(agent, key, ctrl_out, len_out);
}

void sweep_pending_control_agent_api(void)
{
  if (agent == NULL)
    return;
  e2_agent_sweep_pending_control(agent);
}

