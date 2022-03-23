//
// Created by leok on 2022/1/20.
//
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <endian.h>
#include <unistd.h>
#include <stdlib.h>
#include <cJSON.h>

#include "message_control.h"
#include "device_manager.h"
#include "serial_control.h"
#include "mqtt_async_control.h"

static uint8_t g_short_addr[3] = {0};
#define ON  "ON"
#define OFF "OFF"

static void dumpData_leok(const unsigned char *buf, size_t length) {
    printf("leok----dump data: ");
    int n = 0;
    char dat[17] = {0};
    for (size_t i = 0; i < length; ++i) {
        dat[n] = buf[i];
        printf("%02x ", buf[i]);
        if (++n >= 16 || i == length - 1) {
            if (i == length - 1) {
                if (length % 16 != 0) {
                    for (size_t blk = 0; blk < 16 - length % 16; ++blk) {
                        printf("   ");
                    }
                }
            }

            printf("\t");
            for (int k = 0; k < n; k++) {
                if (dat[k] >= 32 && dat[k] <= 126) {
                    printf("%c", dat[k]);
                } else {
                    printf(".");
                }
            }
            printf("\n");
            n = 0;
        }
    }
    printf("\n");
}


static void freeJson(cJSON *json) {
    if (json != NULL) {
        cJSON_Delete(json);
    }
}

//----------------mqtt publish start------------------------------------------------------

#if HOMEASSISTANT_ENABLE

static void switchDiscovery(void *data)
{
    if (NULL == data) {
        printf("%s: input data null\n", __func__);
        return;
    }

    OnlinePublish* online_publish = (OnlinePublish*) data;

    cJSON *item = cJSON_CreateObject();
    char transmit[128] = {0};

    cJSON_AddStringToObject(item, "name", (const char*)online_publish->deviceid);
    cJSON_AddStringToObject(item, "platform", "mqtt");

    memset(transmit, 0, sizeof(transmit));
    snprintf(transmit, sizeof(transmit), "%s-switch", (const char*)online_publish->deviceid);
    cJSON_AddStringToObject(item, "unique_id", transmit);

    memset(transmit, 0, sizeof(transmit));
    snprintf(transmit, sizeof(transmit), "toybrick/switch/%s/set", (const char*)online_publish->deviceid);
    cJSON_AddStringToObject(item, "command_topic", transmit);

    memset(transmit, 0, sizeof(transmit));
    snprintf(transmit, sizeof(transmit), "toybrick/switch/%s/state", (const char*)online_publish->deviceid);
    cJSON_AddStringToObject(item, "state_topic", transmit);

    char *cjson = cJSON_Print(item);
//    printf("json:%s\n", cjson);


    memset(transmit, 0, sizeof(transmit));
    snprintf(transmit, sizeof(transmit), "homeassistant/switch/%s/config", (const char*)online_publish->deviceid);
    mqttMessagePublish(transmit, cjson);


    freeJson(item);

    if (cjson != NULL) {
        free(cjson);
    }
}

static void currentSensorDiscovery(void *data)
{
    if (NULL == data) {
        printf("%s: input data null\n", __func__);
        return;
    }

    OnlinePublish* online_publish = (OnlinePublish*) data;

    cJSON *item = cJSON_CreateObject();
    char transmit[128] = {0};

    cJSON_AddStringToObject(item, "device_class", "current");
    cJSON_AddStringToObject(item, "name", (const char*)online_publish->deviceid);

    cJSON_AddStringToObject(item, "platform", "mqtt");
    memset(transmit, 0, sizeof(transmit));
    snprintf(transmit, sizeof(transmit), "%s-current", (const char*)online_publish->deviceid);
    cJSON_AddStringToObject(item, "unique_id", transmit);

    memset(transmit, 0, sizeof(transmit));
    snprintf(transmit, sizeof(transmit), "toybrick/sensor/%s-VA/state", (const char*)online_publish->deviceid);
    cJSON_AddStringToObject(item, "state_topic", transmit);

    cJSON_AddStringToObject(item, "unit_of_measurement", "A");
    cJSON_AddStringToObject(item, "value_template", "{{value_json.current}}");

    char *cjson = cJSON_Print(item);
//    printf("current json:%s\n", cjson);

    memset(transmit, 0, sizeof(transmit));
    snprintf(transmit, sizeof(transmit), "homeassistant/sensor/%s-A/config", (const char*)online_publish->deviceid);
    mqttMessagePublish(transmit, cjson);

    freeJson(item);

    if (cjson != NULL) {
        free(cjson);
    }
}

static void voltageSensorDiscovery(void *data)
{
    if (NULL == data) {
        printf("%s: input data null\n", __func__);
        return;
    }

    OnlinePublish* online_publish = (OnlinePublish*) data;

    cJSON *item = cJSON_CreateObject();
    char transmit[128] = {0};

    cJSON_AddStringToObject(item, "device_class", "voltage");
    cJSON_AddStringToObject(item, "name", (const char*)online_publish->deviceid);

    cJSON_AddStringToObject(item, "platform", "mqtt");
    memset(transmit, 0, sizeof(transmit));
    snprintf(transmit, sizeof(transmit), "%s-voltage", (const char*)online_publish->deviceid);
    cJSON_AddStringToObject(item, "unique_id", transmit);

    memset(transmit, 0, sizeof(transmit));
    snprintf(transmit, sizeof(transmit), "toybrick/sensor/%s-VA/state", (const char*)online_publish->deviceid);
    cJSON_AddStringToObject(item, "state_topic", transmit);

    cJSON_AddStringToObject(item, "unit_of_measurement", "V");
    cJSON_AddStringToObject(item, "value_template", "{{value_json.voltage}}");

    char *cjson = cJSON_Print(item);
//    printf("voltage json:%s\n", cjson);

    memset(transmit, 0, sizeof(transmit));
    snprintf(transmit, sizeof(transmit), "homeassistant/sensor/%s-V/config", (const char*)online_publish->deviceid);
    mqttMessagePublish(transmit, cjson);

    freeJson(item);

    if (cjson != NULL) {
        free(cjson);
    }
}


static void energySensorDiscovery(void *data)
{
    if (NULL == data) {
        printf("%s: input data null\n", __func__);
        return;
    }

    OnlinePublish* online_publish = (OnlinePublish*) data;

    cJSON *item = cJSON_CreateObject();
    char transmit[128] = {0};

    cJSON_AddStringToObject(item, "device_class", "energy");
    cJSON_AddStringToObject(item, "name", (const char*)online_publish->deviceid);

    cJSON_AddStringToObject(item, "state_class", "total_increasing");

    cJSON_AddStringToObject(item, "platform", "mqtt");
    memset(transmit, 0, sizeof(transmit));
    snprintf(transmit, sizeof(transmit), "%s-energy", (const char*)online_publish->deviceid);
    cJSON_AddStringToObject(item, "unique_id", transmit);

    memset(transmit, 0, sizeof(transmit));
    snprintf(transmit, sizeof(transmit), "toybrick/sensor/%s-kWh/state", (const char*)online_publish->deviceid);
    cJSON_AddStringToObject(item, "state_topic", transmit);

    cJSON_AddStringToObject(item, "unit_of_measurement", "kWh");
    cJSON_AddStringToObject(item, "value_template", "{{value_json.energy}}");

    char *cjson = cJSON_Print(item);
//    printf("energy json:%s\n", cjson);

    memset(transmit, 0, sizeof(transmit));
    snprintf(transmit, sizeof(transmit), "homeassistant/sensor/%s-kWh/config", (const char*)online_publish->deviceid);
    mqttMessagePublish(transmit, cjson);

    freeJson(item);

    if (cjson != NULL) {
        free(cjson);
    }
}



static void multiSensorsDiscovery(void *data)
{
    if (NULL == data) {
        printf("%s: input data null\n", __func__);
        return;
    }

    currentSensorDiscovery(data);
    voltageSensorDiscovery(data);
    energySensorDiscovery(data);

}
#endif

static void onlinePublish(void *data)
{
    if (NULL == data) {
        printf("%s: input data null\n", __func__);
        return;
    }



#if HOMEASSISTANT_ENABLE

    switchDiscovery(data);
    multiSensorsDiscovery(data);

#else

    OnlinePublish* online_publish = (OnlinePublish*) data;
    cJSON *item = cJSON_CreateObject();
    cJSON_AddStringToObject(item, "deviceId", (const char*)online_publish->deviceid);
    cJSON_AddStringToObject(item, "status", online_publish->status);

    char *cjson = cJSON_Print(item);
//    printf("json:%s\n", cjson);

    mqttMessagePublish(ONLINE_TOPIC, cjson);

    freeJson(item);

    if (cjson != NULL) {
        free(cjson);
    }
#endif


}

#if HOMEASSISTANT_ENABLE
static void switchStatePublish(void *data)
{
    if (NULL == data) {
        printf("%s: input data null\n", __func__);
        return;
    }

    if (g_short_addr[1] == 0x00 && g_short_addr[2] == 0x00) {
        printf("get shoet addr failed\n");
        return;
    }

    switchControl *swicth_control = (switchControl *)data;
    char transmit[128] = {0};
    memset(transmit, 0, sizeof(transmit));
    snprintf(transmit, sizeof(transmit), "toybrick/switch/%s/state", (const char*)swicth_control->deviceid);
    if (0 == swicth_control->control) {
        mqttMessagePublish(transmit, OFF);
    } else if (1 == swicth_control->control) {
        mqttMessagePublish(transmit, ON);
    } else {
        printf("switchStatePublish swicth_control->control not 0/1 state\n");
        return;
    }

}

static void voltageCurrentSensorPublish(void *data)
{
    if (NULL == data) {
        printf("%s: input data null\n", __func__);
        return;
    }

    VCPublish* publish = (VCPublish*) data;

    cJSON *item = cJSON_CreateObject();

//    printf("publish->voltage: %s, atof(publish->voltage): %f\n", publish->voltage, atof(publish->voltage));
//    printf("publish->current: %s, atof(publish->current): %f\n", publish->current, atof(publish->current));
    cJSON_AddNumberToObject(item, "voltage", atof(publish->voltage));
    cJSON_AddNumberToObject(item, "current", atof(publish->current)/1000);

    char *cjson = cJSON_Print(item);
//    printf("voltageCurrentSensorPublish json:%s\n", cjson);

    char transmit[128] = {0};
    memset(transmit, 0, sizeof(transmit));
    snprintf(transmit, sizeof(transmit), "toybrick/sensor/%s-VA/state", (const char*)publish->deviceid);
    mqttMessagePublish(transmit, cjson);

    freeJson(item);

    if (cjson != NULL) {
        free(cjson);
    }
}


static void energySensorPublish(void *data)
{
    if (NULL == data) {
        printf("%s: input data null\n", __func__);
        return;
    }

    PCPublish* publish = (PCPublish*) data;

    cJSON *item = cJSON_CreateObject();
    cJSON_AddNumberToObject(item, "energy", atof(publish->power)/1000);

    char *cjson = cJSON_Print(item);
    printf("energySensorPublish json:%s\n", cjson);

    char transmit[128] = {0};
    memset(transmit, 0, sizeof(transmit));
    snprintf(transmit, sizeof(transmit), "toybrick/sensor/%s-kWh/state", (const char*)publish->deviceid);
    mqttMessagePublish(transmit, cjson);

    freeJson(item);

    if (cjson != NULL) {
        free(cjson);
    }
}

#endif


static void voltageCurrentPublish(void *data)
{


#if HOMEASSISTANT_ENABLE
    voltageCurrentSensorPublish(data);
#else

    if (NULL == data) {
        printf("%s: input data null\n", __func__);
        return;
    }

    VCPublish* publish = (VCPublish*) data;

    cJSON *item = cJSON_CreateObject();
    cJSON *properties = cJSON_CreateObject();
    cJSON_AddStringToObject(item, "deviceId", (const char*)publish->deviceid);
    cJSON_AddStringToObject(properties, "voltage", publish->voltage);
    cJSON_AddStringToObject(properties, "current", publish->current);
    cJSON_AddItemToObject(item, "properties", properties);

    char *cjson = cJSON_Print(item);
//    printf("json:%s\n", cjson);

    mqttMessagePublish(REPORT_PROPERTIES_TOPIC, cjson);

    freeJson(item);

    if (cjson != NULL) {
        free(cjson);
    }
#endif
}

static void powerConsumptionPublish(void *data)
{
#if HOMEASSISTANT_ENABLE
    energySensorPublish(data);
#else
    if (NULL == data) {
        printf("%s: input data null\n", __func__);
        return;
    }

    PCPublish* publish = (PCPublish*) data;

    cJSON *item = cJSON_CreateObject();
    cJSON *properties = cJSON_CreateObject();
    cJSON_AddStringToObject(item, "deviceId", (const char*)publish->deviceid);
    cJSON_AddStringToObject(properties, "powerConsumption", publish->power);
    cJSON_AddItemToObject(item, "properties", properties);

    char *cjson = cJSON_Print(item);
//    printf("json:%s\n", cjson);

    mqttMessagePublish(REPORT_PROPERTIES_TOPIC, cjson);

    freeJson(item);

    if (cjson != NULL) {
        free(cjson);
    }
#endif
}


static void softLabelPublish(void *data)
{
    if (NULL == data) {
        printf("%s: input data null\n", __func__);
        return;
    }

    SLPublish* publish = (SLPublish*) data;

    cJSON *item = cJSON_CreateObject();
    cJSON *tags = cJSON_CreateObject();
    cJSON_AddStringToObject(item, "deviceId", (const char*)publish->deviceid);
    cJSON_AddStringToObject(tags, "mac", publish->mac);
    cJSON_AddItemToObject(item, "tags", tags);

    char *cjson = cJSON_Print(item);
    printf("json:%s\n", cjson);

    mqttMessagePublish(TAGS_TOPIC, cjson);

    freeJson(item);

    if (cjson != NULL) {
        free(cjson);
    }
}

//----------------mqtt publish end------------------------------------------------------

void parse_device_online_offline(const void *data)
{
    if (NULL == data) {
        printf("%s: input data null\n", __func__);
        return;
    }
    PayloadPackage* payload = (PayloadPackage*) data;
    MessagePackage *msg_pkg = (MessagePackage *)payload->message;

    uint32_t key = htobe32(msg_pkg->key);
    uint8_t vlength = msg_pkg->vlength;
    uint8_t value = msg_pkg->value[0];
    printf("%s, key: %d vlength: %d value: %d\n", __func__ , key, vlength, value);

    OnlinePublish publish = {0};
    snprintf((char *)publish.deviceid, sizeof(publish.deviceid), "%02x%02x%02x%02x%02x%02x%02x%02x",
             payload->id[0],
             payload->id[1],
             payload->id[2],
             payload->id[3],
             payload->pid[0],
             payload->pid[1],
             payload->vid[0],
             payload->vid[1]);

    int status = (int)value;
    snprintf(publish.status, sizeof(publish.status), "%d", status);
    onlinePublish((void*)&publish);

}

void parse_device_data_upload(const void *data)
{
    if (NULL == data) {
        printf("%s: input data null\n", __func__);
        return;
    }

    PayloadPackage* payload = (PayloadPackage*) data;
    MessagePackage *msg_pkg = (MessagePackage *)payload->message;

    uint32_t key = htobe32(msg_pkg->key);
    uint8_t vlength = msg_pkg->vlength;

    uint8_t voltage = msg_pkg->value[0]; // V
    uint16_t current = msg_pkg->value[2] << 8 | msg_pkg->value[1]; // mA
    printf("%s, key: %d vlength: %d voltage: %d V, current: %d mA\n", __func__ , key, vlength, voltage, current);

    VCPublish publish = {0};
    snprintf((char *)publish.deviceid, sizeof(publish.deviceid), "%02x%02x%02x%02x%02x%02x%02x%02x",
             payload->id[0],
             payload->id[1],
             payload->id[2],
             payload->id[3],
             payload->pid[0],
             payload->pid[1],
             payload->vid[0],
             payload->vid[1]);

    int voltage_transmit = (int)voltage;
    snprintf(publish.voltage, sizeof(publish.voltage), "%d", voltage_transmit);
    int current_transmit = (int)current;
    snprintf(publish.current, sizeof(publish.current), "%d", current_transmit);
    voltageCurrentPublish((void*)&publish);

}

void parse_device_power_consumption(const void *data)
{
    if (NULL == data) {
        printf("%s: input data null\n", __func__);
        return;
    }

    PayloadPackage* payload = (PayloadPackage*) data;
    MessagePackage *msg_pkg = (MessagePackage *)payload->message;

    uint32_t key = htobe32(msg_pkg->key);
    uint8_t vlength = msg_pkg->vlength;
    uint16_t power_consumption = msg_pkg->value[0] << 8 | msg_pkg->value[1];
    printf("%s, key: %d vlength: %d power_consumption: %d\n", __func__ , key, vlength, power_consumption);

    PCPublish publish = {0};
    snprintf((char *)publish.deviceid, sizeof(publish.deviceid), "%02x%02x%02x%02x%02x%02x%02x%02x",
             payload->id[0],
             payload->id[1],
             payload->id[2],
             payload->id[3],
             payload->pid[0],
             payload->pid[1],
             payload->vid[0],
             payload->vid[1]);

    int pc_transmit = (int)power_consumption;
    snprintf(publish.power, sizeof(publish.power), "%d", pc_transmit);
    powerConsumptionPublish((void*)&publish);

}


void parse_device_soft_label(const void *data)
{
    if (NULL == data) {
        printf("%s: input data null\n", __func__);
        return;
    }

    PayloadPackage* payload = (PayloadPackage*) data;
    MessagePackage *msg_pkg = (MessagePackage *)payload->message;

    uint32_t key = htobe32(msg_pkg->key);
    uint8_t vlength = msg_pkg->vlength;
    uint8_t mac[8] = {0};
    memcpy(mac, msg_pkg->value, 8);
    printf("parse_device_soft_label: mac\n");
    dumpData_leok(mac, sizeof(mac));

    DeviceInfo device_info = {0};
    memcpy(device_info.id, payload->id, sizeof(device_info.id));
    memcpy(device_info.pid, payload->pid, sizeof(device_info.pid));
    memcpy(device_info.vid, payload->vid, sizeof(device_info.vid));
    memcpy(device_info.mac, mac, sizeof(device_info.mac));
    addDevice((void *)&device_info);

    printf("%s, key: %d vlength: %d mac: %s\n", __func__ , key, vlength, mac);

    SLPublish publish = {0};
    snprintf((char *)publish.deviceid, sizeof(publish.deviceid), "%02x%02x%02x%02x%02x%02x%02x%02x",
             payload->id[0],
             payload->id[1],
             payload->id[2],
             payload->id[3],
             payload->pid[0],
             payload->pid[1],
             payload->vid[0],
             payload->vid[1]);

    snprintf(publish.mac, sizeof(publish.mac), "%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x",
             mac[0],
             mac[1],
             mac[2],
             mac[3],
             mac[4],
             mac[5],
             mac[6],
             mac[7]);
    softLabelPublish((void*)&publish);
}


void get_short_addr(const void* data)
{
    if (NULL == data) {
        printf("%s: input data null\n", __func__);
        return;
    }

    memset(g_short_addr, 0, sizeof(g_short_addr));
    uint8_t mac[8] = {0};
    memcpy(mac, data, sizeof(mac));
    char send_buff[12] = {0xFE, 0x09, 0x10, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], mac[6], mac[7], 0xFF};
    printf("get_short_addr send_buff: \n");
    dumpData_leok((const unsigned char*)send_buff, sizeof(send_buff));
    int bytes = serialWrite(send_buff, sizeof(send_buff));
    printf("get_short_addr serialWrite bytes: %d\n", bytes);
    sleep(1);
}


static void writeSwitchControl(const void *data)
{
    if (NULL == data) {
        printf("%s: input data null\n", __func__);
        return;
    }

    switchControl *swicth_control = (switchControl *)data;
    printf("%s: dump switchControl\n", __func__ );
    dumpData_leok((const unsigned char*)swicth_control->deviceid, sizeof(swicth_control->deviceid));
    printf("swicth_control->control: %d\n", swicth_control->control);

    int ret = -1;
    uint8_t mac[8] = {0};
    DeviceInfo select_device = {0};
    ret = selectDevice(swicth_control->deviceid, (void *)&select_device);
    if ( DEVICE_OK != ret ) {
        printf("selectDevice ret: %d\n", ret);
        return;
    }
    memcpy(mac, select_device.mac, sizeof(mac));
    get_short_addr(mac);


    if (g_short_addr[1] == 0x00 && g_short_addr[2] == 0x00) {
        printf("get shoet addr failed\n");
        return;
    }
    SwitchPackage switch_package = {0};

    switch_package.head[0] = 0xFC;
    switch_package.head[1] = 0x21;
    switch_package.head[2] = 0x03;
    switch_package.head[3] = 0x01;
    switch_package.head[4] = g_short_addr[1];
    switch_package.head[5] = g_short_addr[2];

    switch_package.rktb[0] = 'R';
    switch_package.rktb[1] = 'K';
    switch_package.rktb[2] = 'T';
    switch_package.rktb[3] = 'B';
    switch_package.total = 25;
    memcpy(switch_package.id, select_device.id, sizeof(switch_package.id));
    memcpy(switch_package.pid, select_device.pid, sizeof(switch_package.pid));
    memcpy(switch_package.vid, select_device.vid, sizeof(switch_package.vid));

    switch_package.mcount = 1;
    switch_package.key = htobe32(CLOUD_SWITCH_CONTROL);
    switch_package.vlength = 1;
    switch_package.value = swicth_control->control;

//    printf("sizeof(switch_package): %lu\n", sizeof(switch_package));
    int bytes = serialWrite((const char *)&switch_package, sizeof(switch_package));
    printf("writeData bytes: %d\n", bytes);
}


void parse_cloud_switch(const void *data)
{

    if (NULL == data) {
        printf("%s: input data null\n", __func__);
        return;
    }

    PublishArrived *pb_arrived = (PublishArrived *)data;
    switchControl swicth_control ={0};

#if HOMEASSISTANT_ENABLE

    if ( 0 == strncmp(pb_arrived->message, ON, sizeof(ON)) ) {
        swicth_control.control = 1;
    } else if(0 == strncmp(pb_arrived->message, OFF, sizeof(OFF))) {
        swicth_control.control = 0;
    } else {
        printf("homeassistant switch set message not ON/OFF\n");
        return;
    }
    memcpy(swicth_control.deviceid, pb_arrived->deviceid, sizeof(swicth_control.deviceid));
    writeSwitchControl((void *)&swicth_control);
    switchStatePublish((void *)&swicth_control);

#else
    cJSON *json_root = cJSON_Parse(pb_arrived->message);
    if (NULL == json_root) {
        printf("cJSON_Parse error:%s\n", cJSON_GetErrorPtr());
        return;
    }

    cJSON *deviceId = cJSON_GetObjectItem(json_root, "deviceId");
    if (NULL == deviceId) {
        printf("deviceId node not exist\n");
        goto FREE_JSON;
    }
    printf("deviceId->valuestring: %s\n", deviceId->valuestring);
    if (NULL == deviceId->valuestring){
        printf("deviceId->valuestring NULL\n");
        goto FREE_JSON;
    }
    memcpy(swicth_control.deviceid, deviceId->valuestring, sizeof(swicth_control.deviceid));

    cJSON *function = cJSON_GetObjectItem(json_root, "function");
    if (NULL == function) {
        printf("function node not exist\n");
        goto FREE_JSON;
    }
    printf("function->valuestring: %s\n", function->valuestring);

    cJSON *args = cJSON_GetObjectItem(json_root, "args");
    if (NULL == args) {
        printf("args node not exist\n");
        goto FREE_JSON;
    }
    printf("args->string: %s\n", args->string);

    int array_size = cJSON_GetArraySize(args);
    printf("array_size: %d\n", array_size);

    for (int cnt = 0; cnt < array_size; cnt++) {
        cJSON *sub = cJSON_GetArrayItem(args, cnt);
        if (NULL == sub) {
            continue;
        }

        cJSON *name = cJSON_GetObjectItem(sub, "name");
        if (NULL == name) {
            printf("name node not exist\n");
            goto FREE_JSON;
        }
        printf("name->valuestring: %s\n", name->valuestring);
        if (NULL == name->valuestring){
            printf("name->valuestring NULL\n");
            goto FREE_JSON;
        }

        if (0 == strncmp(name->valuestring, "switch", sizeof("switch"))) {
            cJSON *value = cJSON_GetObjectItem(sub, "value");
            if (NULL == value) {
                printf("value node not exist\n");
                goto FREE_JSON;
            }
            printf("value->valuestring: %s\n", value->valuestring);
            if (NULL == value->valuestring){
                printf("value->valuestring NULL\n");
                goto FREE_JSON;
            }
            swicth_control.control = atoi(value->valuestring);
            writeSwitchControl((void *)&swicth_control);
        }
    }

#if 0
    cJSON *messageId = cJSON_GetObjectItem(json_root, "messageId");
    if (NULL == messageId) {
        printf("messageId node not exist\n");
        goto FREE_JSON;
    }
    printf("messageId->valuestring: %s\n", messageId->valuestring);
    if (NULL == messageId->valuestring){
        printf("messageId->valuestring NULL\n");
        goto FREE_JSON;
    }
#endif


FREE_JSON:

    freeJson(json_root);
#endif // HOMEASSISTANT_ENABLE
}



void parse_short_addr(const void *data)
{
    if (NULL == data) {
        printf("%s: input data null\n", __func__);
        return;
    }
    memcpy(g_short_addr, data, sizeof(g_short_addr));
    printf("parse_short_addr=======================================: \n");
    dumpData_leok(g_short_addr, sizeof(g_short_addr));

}
