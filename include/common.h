//
// Created by leok on 2022/1/21.
//


#include <stdio.h>
#include <stdint.h>
#include <string.h>

#ifndef PROJECT_COMMON_H
#define PROJECT_COMMON_H
#ifdef __cplusplus
extern "C" {
#endif


typedef struct PayloadPackage {
    uint16_t total;
    uint8_t id[4];
    uint8_t pid[2];
    uint8_t vid[2];
    long long int ts;
    uint8_t  message[];
} PayloadPackage;


typedef struct MessagePackage {
    uint32_t key;
    uint8_t vlength;
    uint8_t  value[];
} MessagePackage;


enum message_key {
    INVALID_MESSAGE = 0,

    TERMINAL_ONLINE_OFFLINE = 0x00000001,
    TERMINAL_DATA_UPLOAD = 0x00000002,
    TERMINAL_POWER_CONSUMPTION = 0x00000003,
    TERMINAL_SOFT_LABEL = 0x00000004,

    TERMINAL_SWITCH_CONTROL = 0x00000005,

};

enum package_error {
    PACKAGE_OK = 0,
    PACKAGE_UNKNOW_MESSAGE,

};

typedef struct TerminalInfo {
    uint8_t id[4];
    uint8_t pid[2];
    uint8_t vid[2];
    uint8_t  mac[8];
} TerminalInfo;

#ifdef __cplusplus
}
#endif
#endif //PROJECT_COMMON_H