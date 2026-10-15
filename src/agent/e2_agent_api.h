/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef E2_AGENT_API_MOSAIC_H
#define E2_AGENT_API_MOSAIC_H

#include "../lib/e2ap/ric_gen_id_wrapper.h"
#include "../sm/sm_io.h"
#include "../util/conf_file.h"
#include "../util/ngran_types.h"

void init_agent_api(int mcc, 
                    int mnc, 
                    int mnc_digit_len,
                    int nb_id,
                    int cu_du_id,
                    ngran_node_t ran_type,
                    sm_io_ag_ran_t io,
                    fr_args_t const* args);

void stop_agent_api(void);

// ric_id must be the full RIC ID of the subscription the indication answers:
// the Requestor ID alone does not identify one (see e2_async_event_agent).
void async_event_agent_api(ric_gen_id_t ric_id, void* ind_data);

#endif

