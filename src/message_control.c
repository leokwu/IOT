//
// Created by leok on 2022/1/20.
//
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "message_control.h"


void parse_online_offline(const void *data)
{
    PayloadPackage* payload = (PayloadPackage*) data;
    MessagePackage *msg_pkg = (MessagePackage *)payload->message;

    uint32_t key = msg_pkg->key;
    uint8_t vlength = msg_pkg->vlength;
    uint8_t value = msg_pkg->value[0];
    printf("%s, key: %d vlength: %d value: %d\n", __func__ , key, vlength, value);
}

void parse_data_upload(const void *data)
{
    PayloadPackage* payload = (PayloadPackage*) data;
    MessagePackage *msg_pkg = (MessagePackage *)payload->message;

    uint32_t key = msg_pkg->key;
    uint8_t vlength = msg_pkg->vlength;
    uint8_t voltage = msg_pkg->value[0]; // V
    uint16_t current = ((uint16_t *) &msg_pkg->value)[1]; // mA
    printf("%s, key: %d vlength: %d voltage: %d V, current: %d mA\n", __func__ , key, vlength, voltage, current);
}

void parse_power_consumption(const void *data)
{
    PayloadPackage* payload = (PayloadPackage*) data;
    MessagePackage *msg_pkg = (MessagePackage *)payload->message;

    uint32_t key = msg_pkg->key;
    uint8_t vlength = msg_pkg->vlength;
    uint32_t power_consumption = msg_pkg->value;
    printf("%s, key: %d vlength: %d power_consumption: %d\n", __func__ , key, vlength, power_consumption);
}


void parse_soft_label(const void *data)
{
    PayloadPackage* payload = (PayloadPackage*) data;
    MessagePackage *msg_pkg = (MessagePackage *)payload->message;

    uint32_t key = msg_pkg->key;
    uint8_t vlength = msg_pkg->vlength;
    uint8_t mac[8] = {0};
    memcpy(mac, msg_pkg->value, 8);
    printf("%s, key: %d vlength: %d mac: %s\n", __func__ , key, vlength, mac);
}