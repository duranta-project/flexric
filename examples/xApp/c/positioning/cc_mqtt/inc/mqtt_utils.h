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

#ifndef MQTT_UTILS_H
#define MQTT_UTILS_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>
#include "mqtt_paramdef.h"
#include "MQTTClient.h"
#include "cjson/cJSON.h"

#include "../../srs_fapi/nfapi_srs_data.h"
#define ADDRESS "tcp://localhost:1883" //"tcp://172.21.16.204:1883"
#define CLIENTID    "xApp"
#define TOPIC       "cc_xapp"



typedef struct {
    char *MqttBrokerAddr;
    char *MqttTopicName;
    char *MqttClientId;
    uint32_t  MqttTrpId;
} mqtt_cfg_t;

__attribute__((always_inline)) inline uint32_t c16amp2(const c16_t a) {
    return a.r * a.r + a.i * a.i;
}
void connect_mqtt(void);

void disconnect_mqtt(void);
// idem to srs_toa_MQTT
void srs_cir_mqtt(c16_t *buffer, uint16_t buf_len, uint16_t xapp_mqtt_id, uint16_t ant_idx);

#if defined(__cplusplus)
}
#endif

#endif