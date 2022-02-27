//
// Created by leok on 2022/1/21.
//

#ifndef PROJECT_PACKAGE_CONTROL_H
#define PROJECT_PACKAGE_CONTROL_H
#ifdef __cplusplus
extern "C" {
#endif
#include "common.h"

//int32_t deserialize_device_message(const void *data);
int32_t deserialize_uart_package(const void *data);
int32_t deserialize_shotaddr_package(const void *data);
int32_t deserialize_cloud_package(char* topicName, int topicLen, int payloadlen, void *payload);


#ifdef __cplusplus
}
#endif
#endif //PROJECT_PACKAGE_CONTROL_H