//
// Created by leok on 2022/1/20.
//
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <endian.h>
#include <unistd.h>
#include <stdlib.h>
#include <cjson/cJSON.h>

#include "message_control.h"
#include "device_manager.h"
#include "serial_control.h"
#include "mqtt_async_control.h"

static uint8_t g_short_addr[3] = {0};

#pragma pack(1)
typedef struct SwitchPackage {
    uint8_t head[6];
    uint8_t rktb[4];
    uint16_t total;
    uint8_t id[4];
    uint8_t pid[2];
    uint8_t vid[2];
    uint8_t ts[8];
    uint8_t mcount;
    uint32_t key;
    uint8_t vlength;
    uint8_t  value;
} SwitchPackage;


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


static void onlinePublish(void *data)
{
    if (NULL == data) {
        printf("%s: input data null\n", __func__);
        return;
    }

    OnlinePublish* online_publish = (OnlinePublish*) data;

    cJSON *item = cJSON_CreateObject();
    cJSON_AddStringToObject(item, "deviceId", online_publish->deviceid);
    cJSON_AddStringToObject(item, "status", online_publish->status);

    char *cjson = cJSON_Print(item);
    printf("json:%s\n", cjson);

    mqttMessagePublish(ONLINE_TOPIC, cjson);

    freeJson(item);

    if (cjson != NULL) {
        free(cjson);
    }
}


static void voltageCurrentPublish(void *data)
{
    if (NULL == data) {
        printf("%s: input data null\n", __func__);
        return;
    }

    VCPublish* vc_publish = (VCPublish*) data;

    cJSON *item = cJSON_CreateObject();
    cJSON_AddStringToObject(item, "deviceId", vc_publish->deviceid);
    cJSON_AddStringToObject(item, "voltage", vc_publish->voltage);
    cJSON_AddStringToObject(item, "current", vc_publish->current);

    char *cjson = cJSON_Print(item);
    printf("json:%s\n", cjson);

    mqttMessagePublish(VOLTAGE_CURRENT_TOPIC, cjson);

    freeJson(item);

    if (cjson != NULL) {
        free(cjson);
    }
}

static void powerConsumptionPublish(void *data)
{
    if (NULL == data) {
        printf("%s: input data null\n", __func__);
        return;
    }

    PCPublish* pc_publish = (PCPublish*) data;

    cJSON *item = cJSON_CreateObject();
    cJSON_AddStringToObject(item, "deviceId", pc_publish->deviceid);
    cJSON_AddStringToObject(item, "powerConsumption", pc_publish->power);

    char *cjson = cJSON_Print(item);
    printf("json:%s\n", cjson);

    mqttMessagePublish(VOLTAGE_CURRENT_TOPIC, cjson);

    freeJson(item);

    if (cjson != NULL) {
        free(cjson);
    }
}



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

    OnlinePublish online_publish = {0};
    snprintf(online_publish.deviceid, sizeof(online_publish.deviceid), "%02x%02x%02x%02x%02x%02x%02x%02x",
             payload->id[0],
             payload->id[1],
             payload->id[2],
             payload->id[3],
             payload->pid[0],
             payload->pid[1],
             payload->vid[0],
             payload->vid[1]);

    int status = (int)value;
    snprintf(online_publish.status, sizeof(online_publish.status), "%d", status);
    onlinePublish((void*)&online_publish);


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
    uint16_t current = msg_pkg->value[1] << 8 | msg_pkg->value[2]; // mA
    printf("%s, key: %d vlength: %d voltage: %d V, current: %d mA\n", __func__ , key, vlength, voltage, current);

    VCPublish vc_publish = {0};
    snprintf(vc_publish.deviceid, sizeof(vc_publish.deviceid), "%02x%02x%02x%02x%02x%02x%02x%02x",
             payload->id[0],
             payload->id[1],
             payload->id[2],
             payload->id[3],
             payload->pid[0],
             payload->pid[1],
             payload->vid[0],
             payload->vid[1]);

    int voltage_transmit = (int)voltage;
    snprintf(vc_publish.voltage, sizeof(vc_publish.voltage), "%d", voltage_transmit);
    int current_transmit = (int)current;
    snprintf(vc_publish.current, sizeof(vc_publish.current), "%d", current_transmit);
    voltageCurrentPublish((void*)&vc_publish);

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

    PCPublish pc_publish = {0};
    snprintf(pc_publish.deviceid, sizeof(pc_publish.deviceid), "%02x%02x%02x%02x%02x%02x%02x%02x",
             payload->id[0],
             payload->id[1],
             payload->id[2],
             payload->id[3],
             payload->pid[0],
             payload->pid[1],
             payload->vid[0],
             payload->vid[1]);

    int pc_transmit = (int)power_consumption;
    snprintf(pc_publish.power, sizeof(pc_publish.power), "%d", pc_transmit);
    powerConsumptionPublish((void*)&pc_publish);

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
//    mac[0] = 0x36;
//    mac[1] = 0x61;
//    mac[2] = 0x6E;
//    mac[3] = 0x24;
//    mac[4] = 0x00;
//    mac[5] = 0x4B;
//    mac[6] = 0x12;
//    mac[7] = 0x00;
    char send_buff[12] = {0xFE, 0x09, 0x10, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], mac[6], mac[7], 0xFF};
    printf("get_short_addr send_buff: \n");
    dumpData_leok(send_buff, sizeof(send_buff));
    int bytes = serialWrite(send_buff, sizeof(send_buff));
    sleep(5);
}


void parse_cloud_switch(const void *data)
{

    if (NULL == data) {
        printf("%s: input data null\n", __func__);
        return;
    }

    int ret = -1;

    uint8_t mac[8] = {0};
    DeviceInfo select_device = {0};
    ret = selectDevice("0000000001010202", (void *)&select_device);
    if ( DEVICE_OK != ret ) {
        printf("selectDevice ret: %d\n", ret);
        return;
    }
    memcpy(mac, select_device.mac, sizeof(mac));
    get_short_addr(mac);

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
    switch_package.value = 2;
//
//
//    dumpData_leok((const unsigned char *)&switch_package, sizeof(switch_package));
//    char send_buff[9] = {0xFC, 0x07, 0x03, 0x01, 0x53, 0x37, 0x31, 0x32, 0x33};
//    serialWrite(send_buff, sizeof(send_buff));

    printf("sizeof(switch_package): %ld\n", sizeof(switch_package));
    int bytes = serialWrite((const char *)&switch_package, sizeof(switch_package));
    printf("writeData bytes: %d\n", bytes);

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
