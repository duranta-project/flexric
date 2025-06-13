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

void srs_cir_mqtt(c16_t *buffer, uint16_t buf_len, uint16_t xapp_mqtt_id, uint16_t ant_idx)
{
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    int rc;
    uint32_t peak_idx = 0;
    uint32_t peak_val = 0;

    cJSON *mqtt_payload = cJSON_CreateObject();
    cJSON_AddNumberToObject(mqtt_payload, "peak_index", peak_idx);
    cJSON_AddNumberToObject(mqtt_payload, "peak_val", peak_val);
    cJSON_AddNumberToObject(mqtt_payload, "source", xapp_mqtt_id);
    cJSON_AddNumberToObject(mqtt_payload, "antenna_index", ant_idx);

    cJSON *chest_json = cJSON_AddArrayToObject(mqtt_payload, "shifted_cir");

    // Temporary array to hold channel estimation values
    int32_t chest_tmp[buf_len];

    // Peak calculation
    uint32_t max_val = 0, max_idx = 0, abs_val = 0;
    for (int k = 0; k < buf_len; k++) {
        abs_val = c16amp2(buffer[k])/2;

        if (abs_val > max_val) {
            max_val = abs_val;
            max_idx = k;
        }

        chest_tmp[k] = abs_val;  // Save to temp array
    }

    peak_idx = max_idx;
    peak_val = max_val;
    printf("peak val=%u\t peak idx=%u\n" , peak_val, peak_idx);

    cJSON_SetIntValue(cJSON_GetObjectItem(mqtt_payload, "peak_index"), peak_idx);
    cJSON_SetIntValue(cJSON_GetObjectItem(mqtt_payload, "peak_val"), peak_val);

    // Circular shift of chest_tmp
    int shift = 2098; 
    int real_size = buf_len; 
    int32_t chest_shifted[real_size];

    for (int i = 0; i < real_size; i++) {
        chest_shifted[i] = chest_tmp[(i - shift + real_size) % real_size];
    }

    int chest_size = 100;
    for (int i = 0; i < chest_size; i++) {
        cJSON_AddItemToArray(chest_json, cJSON_CreateNumber(chest_shifted[i]));
    }

    pubmsg.payload = cJSON_PrintUnformatted(mqtt_payload);
    pubmsg.payloadlen = (int)strlen(pubmsg.payload);
    pubmsg.qos = 0;
    pubmsg.retained = 0;

    if ((rc = MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to publish \"SRS CIR measurements\" MQTT message, return code %d\n", rc);
    }
    printf("MQTT published meas\n");
    cJSON_Delete(mqtt_payload);
}
