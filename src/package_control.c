//
// Created by leok on 2022/1/21.
//

#include <endian.h>
#include <cJSON.h>
#include <stdbool.h>

#include "package_control.h"
#include "message_control.h"
#include "mqtt_async_control.h"

#define SWITCH_FUNCTION "switch"

#if 0
static void dumpData_leok(const unsigned char *buf, size_t length) {
    printf("leok----dump data: ");
    int n = 0;
    char dat[17] = {0};
    for (size_t i = 0; i < length; ++i) {
        dat[n] = buf[i];
        printf("%02x ", buf[i]);
        if (++n >= 16 || i == length - 1) {
            if (i == length - 1) {
                if (length % 16 != 0) {
                    for (size_t blk = 0; blk < 16 - length % 16; ++blk) {
                        printf("   ");
                    }
                }
            }

            printf("\t");
            for (int k = 0; k < n; k++) {
                if (dat[k] >= 32 && dat[k] <= 126) {
                    printf("%c", dat[k]);
                } else {
                    printf(".");
                }
            }
            printf("\n");
            n = 0;
        }
    }
    printf("\n");
}
#endif


// device-----------------------------------------------------------
typedef void (*parse_device_message_func)(const void *data);
struct parse_device_message_map {
    enum device_message_key msg_key;
    parse_device_message_func parse_message;
};

int32_t deserialize_device_message(const void *data)
{
    const struct parse_device_message_map map[] = { { DEVICE_ONLINE_OFFLINE, parse_device_online_offline },
                                             { DEVICE_DATA_UPLOAD, parse_device_data_upload },
                                             { DEVICE_POWER_CONSUMPTION, parse_device_power_consumption },
                                             { DEVICE_SWITCH_STATUS, parse_device_switch_status },
                                             { DEVICE_OVERLOAD_STATUS, parse_device_overload_status },
                                             { DEVICE_SOFT_LABEL, parse_device_soft_label } };


    PayloadPackage* payload = (PayloadPackage*) data;
    MessagePackage *msg_pkg = (MessagePackage *)payload->message;

    for (uint32_t i = 0; i < sizeof(map) / sizeof(struct parse_device_message_map); i++) {
        if (map[i].msg_key != htobe32(msg_pkg->key)) {
            continue;
        }
        printf("deserialize_device_message support parse 0x%08x message\n", htobe32(msg_pkg->key));
        map[i].parse_message(data);
        return PACKAGE_OK;
    }
    printf("deserialize_device_message Unsupport parse 0x%08x message\n", htobe32(msg_pkg->key));
    return PACKAGE_UNKNOW_MESSAGE;
}

int32_t deserialize_uart_package(const void *data)
{

    if (NULL == data) {
        printf("deserialize_uart_package data null\n");
        return PACKAGE_PTR_NULL;
    }
#if 0 // FOR DEBUG
    //    auto payload = (PayloadPackage*) data;
    PayloadPackage* payload = (PayloadPackage*) data;
    uint16_t total = htobe16(payload->total);
    uint8_t id[4] = {0};
    memcpy(id, payload->id, 4);
    uint8_t pid[2] = {0};
    memcpy(pid, payload->pid, 2);
    uint8_t vid[2] = {0};
    memcpy(vid, payload->vid, 2);
    uint8_t ts[8] = {0};
    memcpy(ts, payload->ts, 8);
    uint8_t mcount = payload->mcount;
    printf("%s total: %d, id: %s, pid: %s, vid: %s, mcount: %d\n", __func__ , total, id, pid, vid, mcount);
#endif

    int32_t ret = deserialize_device_message(data);
    printf("%s ret: %d\n", __func__, ret);
    return ret;
}
// device-----------------------------------------------------------


// cloud------------------------------------------------------------
typedef void (*parse_cloud_message_func)(const void *data);
struct parse_cloud_message_map {
    enum cloud_message_key msg_key;
    parse_cloud_message_func parse_message;
};


int32_t deserialize_cloud_message(const void *data)
{
    const struct parse_cloud_message_map map[] = { { CLOUD_SWITCH_CONTROL, parse_cloud_switch } };

    PublishArrived *pb_arrived = (PublishArrived *)data;

    for (uint32_t i = 0; i < sizeof(map) / sizeof(struct parse_cloud_message_map); i++) {
        if (map[i].msg_key != pb_arrived->function) {
            continue;
        }
        printf("deserialize_cloud_message support parse 0x%08x message\n", pb_arrived->function);
        map[i].parse_message(data);
        return PACKAGE_OK;
    }
    printf("deserialize_cloud_message Unsupport parse 0x%08x message\n", pb_arrived->function);
    return PACKAGE_UNKNOW_MESSAGE;
}



#if HOMEASSISTANT_ENABLE
static bool check_switch_set(char *pcBuf, char *pcRes)
{
    char *pcBegin = NULL;
    char *pcEnd = NULL;
 
    pcBegin = strstr(pcBuf, "toybrick/switch/");
    pcEnd = strstr(pcBuf, "/set");
 
    if(pcBegin == NULL || pcEnd == NULL || pcBegin > pcEnd)
    {
        printf("deviceid not found!\n");
        return false;
    } else {
        pcBegin += strlen("toybrick/switch/");
        memcpy(pcRes, pcBegin, pcEnd - pcBegin);
    }
 
    return true;
}

#else
static uint32_t get_function_type(void *data) {

    cJSON *json_root = cJSON_Parse(data);
    if (NULL == json_root) {
        printf("cJSON_Parse error:%s\n", cJSON_GetErrorPtr());
    }

    uint32_t ret = 0;

    cJSON *function = cJSON_GetObjectItem(json_root, "function");
    if (NULL == function) {
        printf("function node not exist\n");
        ret = CLOUD_INVALID_MESSAGE;
        goto FREE_JSON;
    }
    printf("function->valuestring: %s\n", function->valuestring);
    if (NULL == function->valuestring){
        printf("function->valuestring NULL\n");
        goto FREE_JSON;
    }

    if( 0 == strncmp(function->valuestring, SWITCH_FUNCTION, sizeof(SWITCH_FUNCTION)) ) {
        ret = CLOUD_SWITCH_CONTROL;
        goto FREE_JSON;

    } else {
        printf("function->valuestring: %s not adjust %s\n", function->valuestring, SWITCH_FUNCTION);
        ret = CLOUD_INVALID_MESSAGE;
        goto FREE_JSON;
    }

FREE_JSON:

    if (json_root != NULL) {
        cJSON_Delete(json_root);
    }

    return ret;
}
#endif

int32_t deserialize_cloud_package(char* topicName, int topicLen, int payloadlen, void *payload)
{
    if (NULL == topicName || NULL == payload) {
        printf("deserialize_cloud_package data null\n");
        return PACKAGE_PTR_NULL;
    }

    uint32_t ret = 0;

#if HOMEASSISTANT_ENABLE
    //TODO:
    char device_id[64] = {0};
    bool is_switch = check_switch_set(topicName, device_id);
    if (is_switch) {
        printf("device_id: %s\n", device_id);
        PublishArrived pb_arrived = {0};
        pb_arrived.function = CLOUD_SWITCH_CONTROL;
        memcpy(pb_arrived.deviceid, (char*)device_id, sizeof(pb_arrived.deviceid));
        memcpy(pb_arrived.message, (char*)payload, sizeof(pb_arrived.message));
        ret = deserialize_cloud_message((void*)&pb_arrived);
    }
#else
    if( 0 == strncmp(topicName, INVOKE_FUNCTION_TOPIC, topicLen) ) {
        if (payloadlen > ARRIVED_MESSAGE_LEN) {
            printf("message->payloadlen: %d > ARRIVED_MESSAGE_LEN: %d\n", payloadlen, ARRIVED_MESSAGE_LEN);
            return 1;
        }

        ret = get_function_type(payload);
        if (ret == CLOUD_SWITCH_CONTROL) {
            PublishArrived pb_arrived = {0};
            pb_arrived.function = CLOUD_SWITCH_CONTROL;
            memcpy(pb_arrived.message, (char*)payload, sizeof(pb_arrived.message));
            ret = deserialize_cloud_message((void*)&pb_arrived);
        }
    }
#endif
    return ret;
}

// cloud------------------------------------------------------------


// read zigbee info-------------------------------------------------

int32_t deserialize_shotaddr_package(const void *data)
{
    if (NULL == data) {
        printf("deserialize_networking_package data null\n");
        return PACKAGE_PTR_NULL;
    }

    parse_short_addr(data);
    return PACKAGE_OK;

}
// read zigbee info-------------------------------------------------
