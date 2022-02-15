//
// Created by leok on 2022/1/20.
//

#ifndef PROJECT_MESSAGE_CONTROL_H
#define PROJECT_MESSAGE_CONTROL_H
#ifdef __cplusplus
extern "C" {
#endif
#include "common.h"

// device
void parse_device_online_offline(const void *data);
void parse_device_data_upload(const void *data);
void parse_device_power_consumption(const void *data);
void parse_device_soft_label(const void *data);

// cloud
void parse_cloud_switch(const void *data);

// zigbee networking
void parse_short_addr(const void *data);
void get_short_addr(const void* data);

#ifdef __cplusplus
}
#endif


#endif //PROJECT_MESSAGE_CONTROL_H