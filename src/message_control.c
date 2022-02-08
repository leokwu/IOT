//
// Created by leok on 2022/1/20.
//
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "message_control.h"


void parse_online_offline(const struct *MessagePackage)
{
    uint32_t key = MessagePackage->key;
    uint8_t vlength = MessagePackage->vlength;
    uint8_t value = MessagePackage->value;
    printf("%s, key: %d vlength: %d value: %d\n", __func__ , key, vlength, value);
}

void parse_data_upload(const struct *MessagePackage)
{
    uint32_t key = MessagePackage->key;
    uint8_t vlength = MessagePackage->vlength;
    uint8_t voltage = MessagePackage->value[0]; // V
    uint16_t current = ((uint16_t *) &MessagePackage->value)[1]; // mA
    printf("%s, key: %d vlength: %d voltage: %d V, current: %d mA\n", __func__ , key, vlength, voltage, current);
}

void parse_power_consumption(const struct *MessagePackage)
{
    uint32_t key = MessagePackage->key;
    uint8_t vlength = MessagePackage->vlength;
    uint32_t power_consumption = MessagePackage->value;
    printf("%s, key: %d vlength: %d power_consumption: %d\n", __func__ , key, vlength, power_consumption);
}


void parse_soft_label(const struct *MessagePackage)
{
    uint32_t key = MessagePackage->key;
    uint8_t vlength = MessagePackage->vlength;
    uint8_t mac[8] = {0};
    memcpy(mac, MessagePackage->value, 8);
    printf("%s, key: %d vlength: %d mac: %s\n", __func__ , key, vlength, mac);
}