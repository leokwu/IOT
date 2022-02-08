//
// Created by leok on 2022/1/20.
//

#ifndef PROJECT_LIST_CONTROL_H
#define PROJECT_LIST_CONTROL_H
#ifdef __cplusplus
extern "C" {
#endif

#include <common.h>


uint8_t *get_mac_from_list(void *data);
int insert_item_to_list(void *data);

#ifdef __cplusplus
}
#endif


#endif //PROJECT_LIST_CONTROL_H