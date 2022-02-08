//
// Created by leok on 2022/1/20.
//

#ifndef PROJECT_MESSAGE_CONTROL_H
#define PROJECT_MESSAGE_CONTROL_H
#ifdef __cplusplus
extern "C" {
#endif
#include "common.h"

void parse_online_offline(const struct *MessagePackage);
void parse_data_upload(const struct *MessagePackage);
void parse_power_consumption(const struct *MessagePackage);
void parse_soft_label(const struct *MessagePackage);



#ifdef __cplusplus
}
#endif


#endif //PROJECT_MESSAGE_CONTROL_H