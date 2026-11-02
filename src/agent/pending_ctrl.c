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

#include "pending_ctrl.h"
#include "../util/time_now_us.h"

#include <assert.h>
#include <string.h>

void init_pending_ctrl(pending_ctrl_t* p)
{
  assert(p != NULL);
  memset(p->slot, 0, sizeof(p->slot));
  int const rc = pthread_mutex_init(&p->mtx, NULL);
  assert(rc == 0);
}

void free_pending_ctrl(pending_ctrl_t* p)
{
  assert(p != NULL);
  int const rc = pthread_mutex_destroy(&p->mtx);
  assert(rc == 0);
}

bool park_pending_ctrl(pending_ctrl_t* p, uint64_t key, ric_gen_id_t ric_id)
{
  assert(p != NULL);
  /* Key 0 is the "no sequence id assigned" sentinel; a control carrying it has
   * nothing to correlate a later completion against, so it must never be
   * parked. Callers reject such a control outright. */
  assert(key != 0);

  int rc = pthread_mutex_lock(&p->mtx);
  assert(rc == 0);

  bool parked = false;
  for (size_t i = 0; i < PENDING_CTRL_MAX; ++i) {
    if (p->slot[i].occupied)
      continue;
    p->slot[i] = (pending_ctrl_slot_t){
        .occupied = true, .key = key, .ric_id = ric_id, .parked_ts_ns = (int64_t)time_now_ns()};
    parked = true;
    break;
  }

  rc = pthread_mutex_unlock(&p->mtx);
  assert(rc == 0);
  return parked;
}

bool claim_pending_ctrl(pending_ctrl_t* p, uint64_t key, ric_gen_id_t* out_ric_id)
{
  assert(p != NULL);
  assert(out_ric_id != NULL);

  int rc = pthread_mutex_lock(&p->mtx);
  assert(rc == 0);

  bool found = false;
  for (size_t i = 0; i < PENDING_CTRL_MAX; ++i) {
    if (!p->slot[i].occupied || p->slot[i].key != key)
      continue;
    *out_ric_id = p->slot[i].ric_id;
    p->slot[i].occupied = false;
    found = true;
    break;
  }

  rc = pthread_mutex_unlock(&p->mtx);
  assert(rc == 0);
  return found;
}

size_t sweep_pending_ctrl(pending_ctrl_t* p, ric_gen_id_t out[PENDING_CTRL_MAX])
{
  assert(p != NULL);
  assert(out != NULL);

  int64_t const now = (int64_t)time_now_ns();

  int rc = pthread_mutex_lock(&p->mtx);
  assert(rc == 0);

  size_t n = 0;
  for (size_t i = 0; i < PENDING_CTRL_MAX; ++i) {
    if (!p->slot[i].occupied)
      continue;
    if (now - p->slot[i].parked_ts_ns < PENDING_CTRL_TIMEOUT_NS)
      continue;
    out[n++] = p->slot[i].ric_id;
    p->slot[i].occupied = false;
  }

  rc = pthread_mutex_unlock(&p->mtx);
  assert(rc == 0);
  return n;
}
