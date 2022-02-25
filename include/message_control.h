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


#pragma pack(1)
typedef struct OnlinePublish {
    uint8_t deviceid[64];
    char status[2];
} OnlinePublish;


#pragma pack(1)
typedef struct VCPublish {
    uint8_t deviceid[64];
    char voltage[8];
    char current[8];
} VCPublish;

#pragma pack(1)
typedef struct PCPublish {
    uint8_t deviceid[64];
    char power[8];
} PCPublish;


#pragma pack(1)
typedef struct SLPublish {
    uint8_t deviceid[64];
    char mac[32];
} SLPublish;



#pragma pack(1)
typedef struct switchControl {
    char deviceid[64];
    int control;
} switchControl;

#ifdef __cplusplus
}
#endif


#endif //PROJECT_MESSAGE_CONTROL_H