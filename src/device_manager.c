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


static void printJsonObjvalue(const cJSON *json) {
    if (NULL == json) {
        printf("NULL object!\n");
        return;
    }

    switch (json->type) {
        case cJSON_False:
            printf("%s: false\n", json->string);
            break;
        case cJSON_True:
            printf("%s: true\n", json->string);
            break;
        case cJSON_NULL:
            printf("%s: cJSON_NULL\n", json->string);
            break;
        case cJSON_Number:
            printf("%s: %d, %f\n", json->string, json->valueint, json->valuedouble);
            break;
        case cJSON_String:
            printf("%s: %s\n", json->string, json->valuestring);
            break;
        case cJSON_Array:
            printf("%s: cJSON_Array\n", json->string);
            break;
        case cJSON_Object:
            printf("%s: cJSON_Object\n", json->string);
            break;
        default:
            printf("unknown type\n");
            break;
    }
}


static void freeJson(cJSON *json) {
    if (json != NULL) {
        cJSON_Delete(json);
    }
}


static void printfJson(cJSON *json) {
    if (NULL == json) {
        return;
    }
    char *cjson = cJSON_Print(json);
    printf("json:%s\n", cjson);
    free(cjson);
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

    freeJson(root);

    return DEVICE_OK;

}

static int insertDevice(void *data)
{

    //TODO:
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

            freeJson(root);
            return DEVICE_OK;
        }
    } else {
            printf("device list node not exist\n");
            return DEVICE_LIST_NOT_EXIST;
    }
}


static int convertToStructure(void *label, void *data)
{
    cJSON *root = readJsonFile(DEVICE_LIST_FILE);
    printfJson(root);

    cJSON *device_list = cJSON_GetObjectItem(root, "device_list");
    if (NULL != device_list) {
        cJSON *item_name = cJSON_GetObjectItem(device_list, label);
        if (NULL != item_name) {
            //TODO:
            cJSON *device_id = cJSON_GetObjectItem(item_name, "device_id");
            cJSON *device_pid = cJSON_GetObjectItem(item_name, "device_pid");
            cJSON *device_vid = cJSON_GetObjectItem(item_name, "device_vid");
            cJSON *device_mac = cJSON_GetObjectItem(item_name, "device_mac");

            TerminalInfo *device_info = (TerminalInfo *)data;
            memcpy(device_info->id, device_id->valuestring, sizeof(device_info->id));
            dumpData_leok(device_info->id, sizeof(device_info->id));
            memcpy(device_info->pid, device_pid->valuestring, sizeof(device_info->pid));
            dumpData_leok(device_info->pid, sizeof(device_info->pid));
            memcpy(device_info->vid, device_vid->valuestring, sizeof(device_info->vid));
            dumpData_leok(device_info->vid, sizeof(device_info->vid));
            memcpy(device_info->mac, device_mac->valuestring, sizeof(device_info->mac));
            dumpData_leok(device_info->mac, sizeof(device_info->mac));
            /*
            device_info->id = device_id->valuestring;
            device_info->pid = device_pid->valuestring;
            device_info->vid = device_vid->valuestring;
            device_info->mac = device_mac->valuestring;
             */
            return DEVICE_OK;

        } else {
            printf("convertToComment device node not exist\n");
            return DEVICE_EXIST;
        }

    } else {
        printf("convertToComment device_list node not exist\n");
        return DEVICE_LIST_NOT_EXIST;
    }
}

void freeStructure(TerminalInfo *device_info)
{
    if (device_info != NULL) {
        free(device_info);
        device_info = NULL;
    }
}

int addDevice(void *data)
{

    int ret = 0;

    if (data == NULL) {
        printf("add_device input data null\n");
        return DEVICE_PTR_NULL;
    }

    if ((access(DEVICE_LIST_FILE, F_OK)) != -1) {
        ret = insertDevice(data);
    } else {
        ret = firstAddDevice(data);
    }
    return ret;
}

int selectDevice(void *label, void *data)
{
    // TODO:
    int ret = 0;

    if ( NULL == label || NULL == data) {
        printf("select_device input data null\n");
        return DEVICE_PTR_NULL;
    }

    if ((access(DEVICE_LIST_FILE, F_OK)) != -1) { //file exist
        ret = convertToStructure(label, data);
    } else {// not exist
        return DEVICE_DB_FILE_NOT_EXIST;
    }
    
    return ret;
}


