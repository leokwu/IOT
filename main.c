//
// Created by leok on 2022/1/18.
//

#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <sys/file.h>

#include "serial_control.h"
#include "device_manager.h"
#include "package_control.h"



void dumpData(const unsigned char *buf, size_t length) {
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

void *recvSerialDataThread(void *arg)
{
    pthread_detach(pthread_self());
    // int fd = *(int*)arg;

    fd_set recv_fd;
//    struct timeval tv = {5, 0};
    char buff[1024] = {0};
    int recv_len = 0;
    int buff_len = sizeof(buff);
    int g_fd = -1;
    g_fd = serialGetFd();

    while(true) {
        struct timeval tv = {1, 0};
        printf("fd: %d\n", g_fd);
        memset(buff, 0, buff_len);
        FD_ZERO(&recv_fd);
        FD_SET(g_fd, &recv_fd);
        while(FD_ISSET(g_fd, &recv_fd)) {
            if(select(g_fd + 1, &recv_fd, NULL, NULL, &tv) < 0) {
                printf("select error\n");
            } else {
                recv_len = serialRead(buff, buff_len);
                if (recv_len > 0) {
                    printf("serialRead start=====================================================\n");
                    printf("recv_len :%d\n", recv_len);
                    dumpData(buff, recv_len);
                    int head = (int)(buff[0] & 0xFF);
                    printf("head: %02x\n", head);
                    if (head == 0xFB && recv_len == 3) {
                        printf("short addr\n");
                        deserialize_shotaddr_package(buff);
                    } else {
                        deserialize_uart_package(buff);
                    }
                    printf("serialRead end=====================================================\n");
                } else {
                    // printf("funcname:%s line:%d recv_len: %d\n",  __func__, __LINE__, recv_len);
                }
            }
        }
    }
}


void *recvCloudDataThread(void *arg)
{
    pthread_detach(pthread_self());

    //TODO:
    while(1) {
        deserialize_cloud_package("leok");
        sleep(5);
    }

}



/**
*@breif     main()
*/
int main(int argc, char **argv)
{

    serialInit();
    int ret = 0;

    pthread_t serial_tid;
    ret = pthread_create(&serial_tid, NULL, recvSerialDataThread, NULL);

    pthread_t cloud_tid;
    ret = pthread_create(&cloud_tid, NULL, recvCloudDataThread, NULL);



#if 0 // for test
    DeviceInfo device_info = {0};
    uint8_t id[4] = {1, 2, 3, 4};
//    snprintf(ter_info.id, sizeof(ter_info.id), "%s", id);
    memcpy(device_info.id, id, sizeof(device_info.id));
    printf("device_info.id: \n");
    dumpData(device_info.id, 4);

    uint8_t pid[2] = {5, 6};
//    snprintf(ter_info.pid, sizeof(ter_info.pid), "%s", pid);
    memcpy(device_info.pid, pid, sizeof(device_info.pid));
    printf("device_info.pid: \n");
    dumpData(device_info.pid, 2);

    uint8_t vid[2] = {7, 8};
//    snprintf(ter_info.vid, sizeof(ter_info.vid), "%s", vid);
    memcpy(device_info.vid, vid, sizeof(device_info.vid));
    printf("device_info.vid: \n");
    dumpData(device_info.vid, 2);

    uint8_t mac[8] = {11, 22, 33, 44, 55, 66, 77, 88};
//    snprintf(ter_info.mac, sizeof(ter_info.mac), "%s", mac);
    memcpy(device_info.mac, mac, sizeof(device_info.mac));
    printf("device_info.mac: \n");
    dumpData(device_info.mac, 8);

//    int insert_ret =  insert_item_to_list((void *)&ter_info);
    addDevice((void *)&device_info);

    uint8_t label[64] = {0};
    snprintf(label, sizeof(label), "%02x%02x%02x%02x%02x%02x%02x%02x",
             device_info.id[0],
             device_info.id[1],
             device_info.id[2],
             device_info.id[3],
             device_info.pid[0],
             device_info.pid[1],
             device_info.vid[0],
             device_info.vid[1]);


    DeviceInfo select_device = {0};
    selectDevice(label, (void *)&select_device);
    printf("select device after: ");
    dumpData(select_device.id, sizeof(select_device.id));
    dumpData(select_device.pid, sizeof(select_device.pid));
    dumpData(select_device.vid, sizeof(select_device.vid));
    dumpData(select_device.mac, sizeof(select_device.mac));
#endif

    while (getchar() != 'q') {
        usleep(10000);
    }

    serialClose();

    return 0;
}
