//
// Created by leok on 2022/2/9.
//

#include <cJSON.h>
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


const char * base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const char padding_char = '=';
static int base64encode(const uint8_t *sourcedata, size_t srclen, char *base64) {
    int i=0, j=0;
    unsigned char trans_index=0;
    const int datalength = srclen;
    for (; i < datalength; i += 3){
        trans_index = ((sourcedata[i] >> 2) & 0x3f);
        base64[j++] = base64char[(int)trans_index];
        trans_index = ((sourcedata[i] << 4) & 0x30);
        if (i + 1 < datalength){
            trans_index |= ((sourcedata[i + 1] >> 4) & 0x0f);
            base64[j++] = base64char[(int)trans_index];
        }else{
            base64[j++] = base64char[(int)trans_index];
            base64[j++] = padding_char;
            base64[j++] = padding_char;
            break;
        }
        trans_index = ((sourcedata[i + 1] << 2) & 0x3c);
        if (i + 2 < datalength){
            trans_index |= ((sourcedata[i + 2] >> 6) & 0x03);
            base64[j++] = base64char[(int)trans_index];
            trans_index = sourcedata[i + 2] & 0x3f;
            base64[j++] = base64char[(int)trans_index];
        }
        else{
            base64[j++] = base64char[(int)trans_index];
            base64[j++] = padding_char;
            break;
        }
    }
    base64[j] = '\0';
    return j;
}

static const unsigned char pr2six[256] = {
        /* ASCII table */
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
        64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
        64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
};


static long base64decode(uint8_t *bufplain, const void *bufcoded) {
    long nbytesdecoded;
    const unsigned char *bufin;
    unsigned char *bufout;
    long nprbytes;

    bufin = (const unsigned char *) bufcoded;
    while (pr2six[*(bufin++)] <= 63);
    nprbytes = (bufin - (const unsigned char *) bufcoded) - 1;
    nbytesdecoded = ((nprbytes + 3) / 4) * 3;

    bufout = (unsigned char *) bufplain;
    bufin = (const unsigned char *) bufcoded;

    while (nprbytes > 4) {
        *(bufout++) =
                (unsigned char) (pr2six[*bufin] << 2 | pr2six[bufin[1]] >> 4);
        *(bufout++) =
                (unsigned char) (pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2);
        *(bufout++) =
                (unsigned char) (pr2six[bufin[2]] << 6 | pr2six[bufin[3]]);
        bufin += 4;
        nprbytes -= 4;
    }

    /* Note: (nprbytes == 1) would be an error, so just ingore that case */
    if (nprbytes > 1) {
        *(bufout++) =
                (unsigned char) (pr2six[*bufin] << 2 | pr2six[bufin[1]] >> 4);
    }
    if (nprbytes > 2) {
        *(bufout++) =
                (unsigned char) (pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2);
    }
    if (nprbytes > 3) {
        *(bufout++) =
                (unsigned char) (pr2six[bufin[2]] << 6 | pr2six[bufin[3]]);
    }

    *(bufout++) = '\0';
    nbytesdecoded -= (4 - nprbytes) & 3;
    return nbytesdecoded;
}

#if 0
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
#endif

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
    cJSON *json_root = cJSON_Parse((const char *)data);
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

    DeviceInfo *device_info = (DeviceInfo *)data;

    cJSON *root = cJSON_CreateObject();
    cJSON *device_list = cJSON_CreateObject();
    cJSON *item = cJSON_CreateObject();


    char transit[1024] = {0};
    memset(transit, 0, sizeof(transit));
    base64encode(device_info->id, sizeof(device_info->id), transit);
    cJSON_AddStringToObject(item, "device_id", transit);

    memset(transit, 0, sizeof(transit));
    base64encode(device_info->pid, sizeof(device_info->pid), transit);
    cJSON_AddStringToObject(item, "device_pid", transit);

    memset(transit, 0, sizeof(transit));
    base64encode(device_info->vid, sizeof(device_info->vid), transit);
    cJSON_AddStringToObject(item, "device_vid", transit);


    memset(transit, 0, sizeof(transit));
    base64encode(device_info->mac, sizeof(device_info->mac), transit);
    cJSON_AddStringToObject(item, "device_mac", transit);

    char label[64] = {0};
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

    DeviceInfo *device_info = (DeviceInfo *)data;
    char label[64] = {0};
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

            char transit[1024] = {0};
            memset(transit, 0, sizeof(transit));
            base64encode(device_info->id, sizeof(device_info->id), transit);
            cJSON_AddStringToObject(item, "device_id", transit);

            memset(transit, 0, sizeof(transit));
            base64encode(device_info->pid, sizeof(device_info->pid), transit);
            cJSON_AddStringToObject(item, "device_pid", transit);

            memset(transit, 0, sizeof(transit));
            base64encode(device_info->vid, sizeof(device_info->vid), transit);
            cJSON_AddStringToObject(item, "device_vid", transit);

            memset(transit, 0, sizeof(transit));
            base64encode(device_info->mac, sizeof(device_info->mac), transit);
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

            DeviceInfo *device_info = (DeviceInfo *)data;
            uint8_t transit[1024] = {0};
            memset(transit, 0, sizeof(transit));
            base64decode(transit, device_id->valuestring);
            memcpy(device_info->id, transit, sizeof(device_info->id));
            dumpData_leok(device_info->id, sizeof(device_info->id));

            memset(transit, 0, sizeof(transit));
            base64decode(transit, device_pid->valuestring);
            memcpy(device_info->pid, transit, sizeof(device_info->pid));
            dumpData_leok(device_info->pid, sizeof(device_info->pid));

            memset(transit, 0, sizeof(transit));
            base64decode(transit, device_vid->valuestring);
            memcpy(device_info->vid, transit, sizeof(device_info->vid));
            dumpData_leok(device_info->vid, sizeof(device_info->vid));

            memset(transit, 0, sizeof(transit));
            base64decode(transit, device_mac->valuestring);
            memcpy(device_info->mac, transit, sizeof(device_info->mac));
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
            return DEVICE_NOT_EXIST;
        }

    } else {
        printf("convertToComment device_list node not exist\n");
        return DEVICE_LIST_NOT_EXIST;
    }
}

void freeStructure(DeviceInfo *device_info)
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


