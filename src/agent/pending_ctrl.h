/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.openairinterface.org/?page_id=698
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */

#ifndef PENDING_CTRL_AGENT_H
#define PENDING_CTRL_AGENT_H

#include "../lib/e2ap/ric_gen_id_wrapper.h"

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>

/* Controls whose acknowledge the Service Model deferred.
 *
 * Most Service Models apply a control inside on_control() and the agent can
 * acknowledge as soon as that returns. The dApp SM cannot: it forwards the
 * control over E3 and the RAN only learns the outcome once the dApp has
 * answered and the change has reached the air, which is several milliseconds
 * and two thread hops later. Acknowledging on return would tell the xApp
 * "applied" when nothing had been applied yet.
 *
 * So the agent parks the RIC ID here and sends nothing; whoever completes the
 * work calls e2_agent_complete_control() with the same key and the outcome
 * goes out then. The key is supplied by the Service Model (for the dApp SM,
 * the E2SM-DAPP sequence-id), because only the SM can decode it.
 *
 * Bounded and non-allocating: a table this small cannot grow without bound if
 * completions stop arriving, and the sweep below retires whatever ages out. */

#define PENDING_CTRL_MAX 64

/* How long a deferred control may wait before it is answered anyway. Sized
 * well above a normal dApp round trip (hundreds of microseconds) and well
 * below any xApp's own patience. */
#define PENDING_CTRL_TIMEOUT_NS (200 * 1000 * 1000)

typedef struct {
  bool occupied;
  uint64_t key;
  ric_gen_id_t ric_id;
  int64_t parked_ts_ns;
} pending_ctrl_slot_t;

typedef struct {
  pthread_mutex_t mtx;
  pending_ctrl_slot_t slot[PENDING_CTRL_MAX];
} pending_ctrl_t;

void init_pending_ctrl(pending_ctrl_t* p);
void free_pending_ctrl(pending_ctrl_t* p);

/* Park a control. Returns false when the table is full, in which case the
 * caller must acknowledge immediately rather than drop the procedure. */
bool park_pending_ctrl(pending_ctrl_t* p, uint64_t key, ric_gen_id_t ric_id);

/* Claim a parked control. Returns false when the key is unknown, which is the
 * normal outcome for a completion that raced the timeout sweep. */
bool claim_pending_ctrl(pending_ctrl_t* p, uint64_t key, ric_gen_id_t* out_ric_id);

/* Retire everything older than PENDING_CTRL_TIMEOUT_NS into out[], returning
 * how many were retired (at most PENDING_CTRL_MAX). The caller acknowledges
 * them negatively; a dApp that never answers must not wedge the xApp. */
size_t sweep_pending_ctrl(pending_ctrl_t* p, ric_gen_id_t out[PENDING_CTRL_MAX]);

#endif
