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


typedef void (*parse_message_func)(const void *data);
struct parse_message_map {
    enum message_key msg_key;
    parse_message_func parse_message;
};

int32_t deserialize_message(const void *data)
{
    const struct parse_message_map map[] = { { TERMINAL_ONLINE_OFFLINE, parse_online_offline },
                                             { TERMINAL_DATA_UPLOAD, parse_data_upload },
                                             { TERMINAL_POWER_CONSUMPTION, parse_power_consumption },
                                             { TERMINAL_SOFT_LABEL, parse_soft_label } };


    PayloadPackage* payload = (PayloadPackage*) data;
    MessagePackage *msg_pkg = (MessagePackage *)payload->message;

    for (uint32_t i = 0; i < sizeof(map) / sizeof(struct parse_message_map); i++) {
        if (map[i].msg_key != htobe32(msg_pkg->key)) {
            continue;
        }
        printf("support parse 0x%08x message\n", htobe32(msg_pkg->key));
        map[i].parse_message(data);
        return PACKAGE_OK;
    }
    printf("Unsupport parse 0x%08x message\n", htobe32(msg_pkg->key));
    return PACKAGE_UNKNOW_MESSAGE;
}

int32_t deserialize_uart_package(const void *data)
{
//    auto payload = (PayloadPackage*) data;
    PayloadPackage* payload = (PayloadPackage*) data;
#if 1
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
    dumpData_leok(ts, sizeof(ts));
//    MessagePackage *message = (MessagePackage *)payload->message;
//    int32_t ret = deserialize_message(message);
#endif
    int32_t ret = deserialize_message(data);
    printf("%s ret: %d\n", __func__, ret);
    return ret;
}
