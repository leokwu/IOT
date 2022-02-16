//
// Created by leok on 2022/1/21.
//
#include "package_control.h"
#include "message_control.h"
#include <endian.h>



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
    //    auto payload = (PayloadPackage*) data;
    PayloadPackage* payload = (PayloadPackage*) data;
#if 1 // FOR DEBUG
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

    for (uint32_t i = 0; i < sizeof(map) / sizeof(struct parse_cloud_message_map); i++) {
        if (map[i].msg_key != CLOUD_SWITCH_CONTROL) {
            continue;
        }
        printf("deserialize_cloud_message support parse 0x%08x message\n", CLOUD_SWITCH_CONTROL);
        map[i].parse_message(data);
        return PACKAGE_OK;
    }
    printf("deserialize_cloud_message Unsupport parse 0x%08x message\n", CLOUD_SWITCH_CONTROL);
    return PACKAGE_UNKNOW_MESSAGE;
}


int32_t deserialize_cloud_package(const void *data)
{
    if (NULL == data) {
        printf("deserialize_cloud_package data null\n");
        return PACKAGE_PTR_NULL;
    }
    int32_t ret = 0;
    ret = deserialize_cloud_message(data);
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