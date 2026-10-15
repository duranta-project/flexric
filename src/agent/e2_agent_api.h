/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef E2_AGENT_API_MOSAIC_H
#define E2_AGENT_API_MOSAIC_H

#include "../sm/sm_io.h"
#include "../util/conf_file.h"
#include "../util/ngran_types.h"
#include <stdint.h>

void init_agent_api(int mcc, 
                    int mnc, 
                    int mnc_digit_len,
                    int nb_id,
                    int cu_du_id,
                    ngran_node_t ran_type,
                    sm_io_ag_ran_t io,
                    fr_args_t const* args);

void stop_agent_api(void);

void async_event_agent_api(uint32_t ric_req_id, void* ind_data);

void trigger_ric_service_update_api(uint16_t ran_func_id);

/* Complete a control the Service Model deferred; see e2_agent_complete_control().
 * Ownership of ctrl_out passes to the agent. Safe from any thread. */
void complete_control_agent_api(uint64_t key, uint8_t* ctrl_out, size_t len_out);

/* Answer every deferred control that has outstayed its budget. Safe from any
 * thread; a deployment that never calls it leaks the pending table's slots to
 * controls nobody completed. */
void sweep_pending_control_agent_api(void);

#endif

