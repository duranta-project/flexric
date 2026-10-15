/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef E2_AGENT_H
#define E2_AGENT_H

#include "util/alg_ds/ds/assoc_container/assoc_generic.h"
#include "util/alg_ds/ds/assoc_container/bimap.h"
#include "util/alg_ds/ds/tsq/tsq.h"

#include "util/conf_file.h"
#include "util/ngran_types.h"

#include "asio_agent.h"
#include "e2ap_agent.h"
#include "endpoint_agent.h"
#include "plugin_agent.h"
#include "sm/sm_io.h"
#include "pending_ctrl.h"
#include "../lib/ctrl_ack_event.h"

#include <stdatomic.h>
#include <stdbool.h>

#ifdef E2AP_V1
#define NUM_HANDLE_MSG 31
#elif E2AP_V2 
#define NUM_HANDLE_MSG 34
#elif E2AP_V3 
#define NUM_HANDLE_MSG 43
#else
static_assert(0!=0, "Unknown E2AP version");
#endif


typedef struct e2_agent_s e2_agent_t;

typedef e2ap_msg_t (*handle_msg_fp_agent)(struct e2_agent_s*, const e2ap_msg_t* msg) ;

typedef struct e2_agent_s 
{
  e2ap_ep_ag_t ep; 
  e2ap_agent_t ap;
  asio_agent_t io;

  size_t sz_handle_msg;
  handle_msg_fp_agent handle_msg[NUM_HANDLE_MSG]; // 26 E2AP + 4 E42AP note that not all the slots will be occupied

  // Registered SMs
  plugin_ag_t plugin;

  // Registered Periodic Indication events
  pthread_mutex_t mtx_ind_event;
  bi_map_t ind_event; // key1:int fd, key2:ind_event_t 

  // Pending events
  bi_map_t pending;  // left: fd, right: pending_event_t 

  global_e2_node_id_t global_e2_node_id;

  // Aperiodic Indication events
  tsq_t aind; // aind_event_t Events that occurred 

  // Control acknowledges the Service Model deferred (see pending_ctrl.h). The
  // table holds the parked RIC IDs; the queue holds the outcomes that are ready
  // to go out, pushed from whatever thread completed the work.
  pending_ctrl_t pending_ctrl;
  tsq_t ctrl_ack; // ctrl_ack_event_t

#if defined(E2AP_V2) || defined (E2AP_V3)
  // Read RAN 
  void (*read_setup_ran)(void* data, const ngran_node_t node_type);
  _Atomic uint32_t trans_id_setup_req;
#endif

  atomic_bool stop_token;
  atomic_bool agent_stopped;
} e2_agent_t;

e2_agent_t* e2_init_agent(const char* addr, int port, global_e2_node_id_t ge2nid, sm_io_ag_ran_t io, char const*  libs_dir);

// Blocking call
void e2_start_agent(e2_agent_t* ag);

void e2_free_agent(e2_agent_t* ag);
     
void e2_async_event_agent(e2_agent_t* ag, uint32_t ric_req_id, void* ind_data);

/* Complete a control whose acknowledge the Service Model deferred, from any
 * thread. key is the one the SM parked it under (for the dApp SM, the
 * E2SM-DAPP sequence-id); ctrl_out is the encoded outcome and ownership passes
 * to the agent, which frees it once the acknowledge is on the wire. A key that
 * is not parked -- already timed out, or never deferred -- is dropped, so a
 * late completion is harmless. */
void e2_agent_complete_control(e2_agent_t* ag, uint64_t key, uint8_t* ctrl_out, size_t len_out);

/* Answer every deferred control that has outstayed PENDING_CTRL_TIMEOUT_NS
 * with an outcome-less acknowledge. Safe to call from any thread and at any
 * cadence; a deployment that never calls it simply leaks the table's 64 slots
 * to controls that were never completed. */
void e2_agent_sweep_pending_control(e2_agent_t* ag);

///////////////////////////////////////////////
// E2AP AGENT FUNCTIONAL PROCEDURES MESSAGES //
///////////////////////////////////////////////

void e2_send_subscription_response(e2_agent_t* ag, const ric_subscription_response_t* sr);

void e2_send_subscription_failure(e2_agent_t* ag, const ric_subscription_failure_t* sf);

void e2_send_subscription_delete_response(e2_agent_t* ag, const ric_subscription_delete_response_t* sdr);

void e2_send_subscription_delete_failure(e2_agent_t* ag, const ric_subscription_delete_failure_t* sdf);

void e2_send_indication_agent(e2_agent_t* ag, const ric_indication_t* indication);

void e2_send_control_acknowledge(e2_agent_t* ag, const ric_control_acknowledge_t* ca);

void e2_send_control_failure(e2_agent_t* ag, const ric_control_failure_t* cf);

void e2_send_ric_service_update(e2_agent_t* ag, const ric_service_update_t* su);

////////////////////////////////////////////////

#undef NUM_HANDLE_MSG 

#endif

