//
// Created by leok on 2022/2/22.
//

#ifndef PROJECT_MQTT_ASYNC_CONTROL_H
#define PROJECT_MQTT_ASYNC_CONTROL_H
#ifdef __cplusplus
extern "C" {
#endif

#include <common.h>

#define ONLINE_TOPIC                "/device_online_status"
#define VOLTAGE_CURRENT_TOPIC       "/device_voltage_current"
#define POWER_CONSUMPTION_TOPIC     "/device_power_consumption"
#define SOFT_LABEL_TOPIC            "/device_soft_label"
#define SWITCH_CONTROL_TOPIC         "/device_switch_control"

int mqttMainProcess();
void mqttMessagePublish(const char* topic, void *payload);
void mqttMessageSubscribe(const char* topic);

#ifdef __cplusplus
}
#endif


#endif //PROJECT_MQTT_ASYNC_CONTROL_H