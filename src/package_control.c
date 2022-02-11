//
// Created by leok on 2022/1/21.
//
#include "package_control.h"
#include "message_control.h"

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
        if (map[i].msg_key != msg_pkg->key) {
            continue;
        }
        map[i].parse_message(data);
        printf("support parse 0x%04x message\n", msg_pkg->key);
        return PACKAGE_OK;
    }
    printf("Unsupport parse 0x%04x message\n", msg_pkg->key);
    return PACKAGE_UNKNOW_MESSAGE;
}

int32_t deserialize_uart_package(const void *data)
{
//    auto payload = (PayloadPackage*) data;
    PayloadPackage* payload = (PayloadPackage*) data;
#if 0
    uint16_t total = payload->total;
    uint8_t id[4] = {0};
    memcpy(id, payload->id, 4);
    uint8_t pid[2] = {0};
    memcpy(pid, payload->pid, 2);
    uint8_t vid[2] = {0};
    memcpy(vid, payload->vid, 2);
    long long int ts = payload->ts;
    uint8_t mcount = payload->mcount;
    printf("%s total: %d, id: %s, pid: %s, vid: %s, mcount: %d, ts: %lld\n", __func__ , total, id, pid, vid, mcount, ts);
    MessagePackage *message = (MessagePackage *)payload->message;
    int32_t ret = deserialize_message(message);
#endif
    int32_t ret = deserialize_message(data);
    printf("%s ret: %d\n", __func__, ret);
    return ret;
}
