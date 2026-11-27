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

#ifndef CTRL_ACK_EVENT_AGENT_H
#define CTRL_ACK_EVENT_AGENT_H

#include "e2ap/ric_gen_id_wrapper.h"

#include <stddef.h>
#include <stdint.h>

/* A deferred control acknowledge that is ready to be sent.
 *
 * ctrl_out is the already-encoded E2SM control outcome, allocated by the
 * completing thread and owned by the event loop, which frees it after the
 * acknowledge goes out. NULL with size 0 is legitimate: a timed-out control is
 * answered with an outcome-less acknowledge. */
typedef struct {
  ric_gen_id_t ric_id;
  uint8_t* ctrl_out;
  size_t len_out;
} ctrl_ack_event_t;

typedef struct {
  ctrl_ack_event_t* arr;
  size_t len;
} arr_ctrl_ack_event_t;

#endif
