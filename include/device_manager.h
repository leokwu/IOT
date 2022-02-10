//
// Created by leok on 2022/2/9.
//

#ifndef PROJECT_DEVICE_MANAGER_H
#define PROJECT_DEVICE_MANAGER_H
#ifdef __cplusplus
extern "C" {
#endif

#include <common.h>

int addDevice(void *data);
TerminalInfo *selectDevice(void *data);
void freeStructure(TerminalInfo *device_info);

#ifdef __cplusplus
}
#endif


#endif //PROJECT_DEVICE_MANAGER_H