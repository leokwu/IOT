//
// Created by leok on 2022/2/22.
//

#ifndef PROJECT_MQTT_ASYNC_CONTROL_H
#define PROJECT_MQTT_ASYNC_CONTROL_H
#ifdef __cplusplus
extern "C" {
#endif

#include <common.h>

int mqttMainProcess();
void mqttMessagePublish(const char* topic, void *payload);
void mqttMessageSubscribe(const char* topic);

#ifdef __cplusplus
}
#endif


#endif //PROJECT_MQTT_ASYNC_CONTROL_H