//
// Created by leok on 2022/2/9.
//

#include <cjson/cJSON.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>


#include "device_manager.h"


#define DEVICE_LIST_FILE "./device_list.json"



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


static cJSON* readJsonFile(char *fileName) {
    if (NULL == fileName) {
        return NULL;
    }

    FILE *fp = NULL;
    uint8_t *data = NULL;

    fp = fopen(fileName, "r");
    if (NULL != fp) {
        fseek(fp, 0, SEEK_END);
        long len = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        data = (uint8_t*)malloc(len + 1);
        fread(data, 1, len, fp);
        fclose(fp);
    }
    
    printf("readJsonFile data:%s\n", data);
    cJSON *json_root = cJSON_Parse(data);
    if (NULL == json_root) {
        printf("cJSON_Parse error:%s\n", cJSON_GetErrorPtr());
    }

    if (NULL != data) {
        free(data);
    }

    return json_root;
}


static void writeJsonFile(char *fileName, cJSON *json) {
    if (NULL == json || NULL == fileName) {
        return;
    }

    char *cjson = cJSON_Print(json);

    FILE *fp = NULL;
    fp = fopen(fileName, "w+");
    if (NULL != fp) {
        fwrite(cjson, strlen(cjson), 1, fp);
        fclose(fp);
    }

    if (NULL != cjson) {
        free(cjson);
    }
}

static int firstAddDevice(void *data)
{

    TerminalInfo *device_info = (TerminalInfo *)data;

    cJSON *root = cJSON_CreateObject();
    cJSON *device_list = cJSON_CreateObject();
    cJSON *item = cJSON_CreateObject();


    uint8_t transit[16] = {0};
    memset(transit, 0, 16);
    memcpy(transit, device_info->id, sizeof(device_info->id));
    cJSON_AddStringToObject(item, "device_id", transit);

    memset(transit, 0, 16);
    memcpy(transit, device_info->pid, sizeof(device_info->pid));
    cJSON_AddStringToObject(item, "device_pid", transit);

    memset(transit, 0, 16);
    memcpy(transit, device_info->vid, sizeof(device_info->vid));
    cJSON_AddStringToObject(item, "device_vid", transit);

    memset(transit, 0, 16);
    memcpy(transit, device_info->mac, sizeof(device_info->mac));
    cJSON_AddStringToObject(item, "device_mac", transit);

    uint8_t label[64] = {0};
    snprintf(label, sizeof(label), "%02x%02x%02x%02x%02x%02x%02x%02x",
             device_info->id[0],
             device_info->id[1],
             device_info->id[2],
             device_info->id[3],
             device_info->pid[0],
             device_info->pid[1],
             device_info->vid[0],
             device_info->vid[1]);

    cJSON_AddItemToObject(device_list, label, item);
    cJSON_AddItemToObject(root, "device_list", device_list);

    writeJsonFile(DEVICE_LIST_FILE, root);

    cJSON_Delete(root);

    return DEVICE_OK;

}

static int insertDevice(void *data)
{

    //TO DO:
    cJSON *root = readJsonFile(DEVICE_LIST_FILE);

    TerminalInfo *device_info = (TerminalInfo *)data;
    uint8_t label[64] = {0};
    snprintf(label, sizeof(label), "%02x%02x%02x%02x%02x%02x%02x%02x",
             device_info->id[0],
             device_info->id[1],
             device_info->id[2],
             device_info->id[3],
             device_info->pid[0],
             device_info->pid[1],
             device_info->vid[0],
             device_info->vid[1]);

    cJSON *device_list = cJSON_GetObjectItem(root, "device_list");
    if (NULL != device_list) {
        cJSON *item_name = cJSON_GetObjectItem(device_list, label);
        if (NULL != item_name) {
            printf("device exist\n");
            return DEVICE_EXIST;
        } else {
            printf("device not exist && insert\n");
            cJSON *item = cJSON_CreateObject();

            uint8_t transit[16] = {0};
            memset(transit, 0, 16);
            memcpy(transit, device_info->id, sizeof(device_info->id));
            cJSON_AddStringToObject(item, "device_id", transit);

            memset(transit, 0, 16);
            memcpy(transit, device_info->pid, sizeof(device_info->pid));
            cJSON_AddStringToObject(item, "device_pid", transit);

            memset(transit, 0, 16);
            memcpy(transit, device_info->vid, sizeof(device_info->vid));
            cJSON_AddStringToObject(item, "device_vid", transit);

            memset(transit, 0, 16);
            memcpy(transit, device_info->mac, sizeof(device_info->mac));
            cJSON_AddStringToObject(item, "device_mac", transit);

            cJSON_AddItemToObject(device_list, label, item);

            writeJsonFile(DEVICE_LIST_FILE, root);

            cJSON_Delete(root);
            return DEVICE_OK;
        }
    } else {
            printf("device list not exist\n");
            return DEVICE_LIST_NOT_EXIST;
    }
}


int add_device(void *data)
{

    if (data == NULL) {
        printf("input data null\n");
        return DEVICE_PTR_NULL;
    }

    if ((access(DEVICE_LIST_FILE, F_OK)) != -1) {
        insertDevice(data);
    } else {
        firstAddDevice(data);
    }
}

int select_device(void *data)
{


}
