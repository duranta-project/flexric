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

#include "../inc/mqtt_utils.h"


MQTTClient client;
mqtt_cfg_t       mqtt_cfg;

MQTTClient client;

void connect_mqtt(void) {
    int rc;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

    if ((rc = MQTTClient_create(&client, ADDRESS, CLIENTID,MQTTCLIENT_PERSISTENCE_NONE, NULL))!= MQTTCLIENT_SUCCESS) {
        fprintf(stderr, "Failed to create MQTT client %s, rc=%d\n", CLIENTID, rc);
        exit(EXIT_FAILURE);
    }

    conn_opts.keepAliveInterval = 60;
    conn_opts.cleansession     = 1;

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        fprintf(stderr, "Failed to connect MQTT client %s, rc=%d\n", CLIENTID, rc);
        exit(EXIT_FAILURE);
    }

    printf("MQTT Connected: client=%s, broker=%s\n", CLIENTID, ADDRESS);
}

void disconnect_mqtt(void) {
    int rc = MQTTClient_disconnect(client, 10000);
    if (rc != MQTTCLIENT_SUCCESS) {
        fprintf(stderr, "Failed to disconnect MQTT client %s, return code %d\n", CLIENTID, rc);
    }
    MQTTClient_destroy(&client);
    printf("MQTT Disconnected.\n");
}

static void channel_amp2(const c16_t *srs_cir, uint16_t Nfft, uint32_t cir_amp2[Nfft])
{
  for(size_t i = 0; i < Nfft; i++){
    cir_amp2[i] = c16amp2(srs_cir[i]);
  }
}

void srs_cir_mqtt(c16_t *buffer, uint16_t buf_len, uint16_t xapp_mqtt_id, uint16_t ant_idx)
{
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    int rc;

    cJSON *mqtt_payload = cJSON_CreateObject();
    cJSON_AddNumberToObject(mqtt_payload, "source", xapp_mqtt_id);
    cJSON_AddNumberToObject(mqtt_payload, "antenna_index", ant_idx);

    cJSON *channel_meas_json = cJSON_AddArrayToObject(mqtt_payload, "srs_cir");

    uint32_t cir_amp2[buf_len];
    channel_amp2(buffer, buf_len, cir_amp2);

    for (int i = 0; i < buf_len; i++) {
        cJSON_AddItemToArray(channel_meas_json, cJSON_CreateNumber(cir_amp2[i]));
    }

    pubmsg.payload = cJSON_PrintUnformatted(mqtt_payload);
    pubmsg.payloadlen = (int)strlen(pubmsg.payload);
    pubmsg.qos = 0;
    pubmsg.retained = 0;

    if ((rc = MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to publish \"SRS Channel measurements\" MQTT message, return code %d\n", rc);
    }
    printf("MQTT published meas\n");
    cJSON_Delete(mqtt_payload);
}
