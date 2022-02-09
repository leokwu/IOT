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


static size_t get_file_size(const char *filepath)
{
    if(NULL == filepath)
        return 0;
    struct stat filestat;
    memset(&filestat, 0, sizeof(struct stat));
    if(0 == stat(filepath, &filestat)) {
        printf("filestat.st_size: %ld\n", filestat.st_size);
        return filestat.st_size;
    } else {
        printf("file size: 0\n");
        return 0;
    }

}

static char *read_file_to_buf(const char *filepath)
{
    if(NULL == filepath)
    {
        return NULL;
    }
    size_t size = get_file_size(filepath);
    if(0 == size)
        return NULL;

    char *buf = malloc(size + 1);
    if(NULL == buf) {
        printf("malloc error\n");
        return NULL;
    }
    memset(buf, 0, size+1);

    FILE *fp = fopen(filepath, "r");
    size_t readSize = fread(buf, 1, size, fp);
    if(readSize != size)
    {
        printf("readSize: %ld != size: %ld", readSize, size);
        free(buf);
        buf = NULL;
        if (fp) {
            fclose(fp);
            fp = NULL;
        }
        return NULL;
    }
    buf[size] = 0;

    if (fp) {
        fclose(fp);
        fp = NULL;
    }

    return buf;
}


static cJSON *prepare_parse_json(const char *filePath)
{
    if(NULL == filePath)
    {
        printf("input para is NULL\n");
        return NULL;
    }
    char *buf = read_file_to_buf(filePath);
    if (NULL == buf) {
        printf("read file to buf failed\n");
        return NULL;
    }

    cJSON *pTemp = cJSON_Parse(buf);
    if (buf) {
        free(buf);
        buf = NULL;
    }

    return pTemp;
}

static int firstadd_device(void *data)
{

    TerminalInfo *device_info = (TerminalInfo *)data;

    cJSON *root = cJSON_CreateObject();
    cJSON *device_list = cJSON_CreateObject();
    cJSON *item = cJSON_CreateObject();


    uint8_t transit[16] = {0};
    memset(transit, 0, 16);
    memcpy(transit, device_info->id, sizeof(device_info->id));
    cJSON_AddRawToObject(item, "device_id", transit);

    memset(transit, 0, 16);
    memcpy(transit, device_info->pid, sizeof(device_info->pid));
    cJSON_AddRawToObject(item, "device_pid", transit);

    memset(transit, 0, 16);
    memcpy(transit, device_info->vid, sizeof(device_info->vid));
    cJSON_AddRawToObject(item, "device_vid", transit);

    memset(transit, 0, 16);
    memcpy(transit, device_info->mac, sizeof(device_info->mac));
    cJSON_AddRawToObject(item, "device_mac", transit);

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

    char *json_data = cJSON_Print(root);
    FILE *fp = fopen(DEVICE_LIST_FILE, "w+");
    if (fp == NULL) {
        cJSON_Delete(root);
        printf("open json file failed\n");
        return DEVICE_OPEN_FAIL;
    }

    fwrite(json_data, sizeof(char), strlen(json_data) + 1, fp);
    dumpData_leok(json_data, strlen(json_data) + 1);

    if (fp) {
        fclose(fp);
        fp = NULL;
    }

    if (json_data) {
        cJSON_free(json_data);
    }

    cJSON_Delete(root);

    return DEVICE_OK;

}

static int insert_device(void *data)
{

    //TO DO:
    cJSON *root = prepare_parse_json(DEVICE_LIST_FILE);

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

    cJSON *device_list = cJSON_GetObjectItem(root, label);
    if (NULL != device_list) {
        cJSON *item_name = cJSON_GetObjectItem(device_list, label);
        if (NULL != item_name) {
            printf("device exist\n");
            return DEVICE_EXIST;
        } else {
            cJSON *item = cJSON_CreateObject();

            uint8_t transit[16] = {0};
            memset(transit, 0, 16);
            memcpy(transit, device_info->id, sizeof(device_info->id));
            cJSON_AddRawToObject(item, "device_id", transit);

            memset(transit, 0, 16);
            memcpy(transit, device_info->pid, sizeof(device_info->pid));
            cJSON_AddRawToObject(item, "device_pid", transit);

            memset(transit, 0, 16);
            memcpy(transit, device_info->vid, sizeof(device_info->vid));
            cJSON_AddRawToObject(item, "device_vid", transit);

            memset(transit, 0, 16);
            memcpy(transit, device_info->mac, sizeof(device_info->mac));
            cJSON_AddRawToObject(item, "device_mac", transit);

            cJSON_AddItemToObject(root, label, item);

            char *json_data = cJSON_Print(root);
            FILE *fp = fopen(DEVICE_LIST_FILE, "w+");
            if (fp == NULL) {
                cJSON_Delete(root);
                printf("open json file failed\n");
                return DEVICE_OPEN_FAIL;
            }

            fwrite(json_data, sizeof(char), strlen(json_data) + 1, fp);
            dumpData_leok(json_data, strlen(json_data) + 1);

            if (fp) {
                fclose(fp);
                fp = NULL;
            }

            if (json_data) {
                cJSON_free(json_data);
            }

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
        insert_device(data);
    } else {
        firstadd_device(data);
    }
}

int select_device(void *data)
{


}
