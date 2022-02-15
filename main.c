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

static int g_fd = -1;

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

void *recvDataThread(void *arg)
{
    pthread_detach(pthread_self());
    // int fd = *(int*)arg;

    fd_set recv_fd;
//    struct timeval tv = {5, 0};
    char buff[1024] = {0};
    int recv_len = 0;
    int buff_len = sizeof(buff);

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
                recv_len = readData(g_fd, buff, buff_len);
                if (recv_len > 0) {
                    printf("recv_len :%d\n", recv_len);
                    dumpData(buff, recv_len);
//                    char send_buff[7] = {0xFC, 0x05, 0x01, 0x02, 0x31, 0x32, 0x33};
                    int head = (int)(buff[0] & 0xFF);
                    printf("head: %02x\n", head);
                    if (head == 0xFB && recv_len == 3) {
                        printf("short addr\n");
                        deserialize_networking_package(buff);
                        deserialize_cloud_package("leok");
//                        exit(1);
//                        sleep(60);
//                        char send_buff[9] = {0xFC, 0x07, 0x03, 0x01, 0x45, 0xF0, 0x31, 0x32, 0x33};
//                        int bytes = writeData(g_fd, send_buff, sizeof(send_buff));
//                        printf("bytes: %d\n", bytes);
                    } else {
                        deserialize_uart_package(buff);
//                        deserialize_cloud_package("leok");
                    }
//                    char send_buff[12] = {0xFE, 0x09, 0x10, 0x36, 0x61, 0x6E, 0x24, 0x00, 0x4B, 0x12, 0x00, 0xFF};
//                    int bytes = writeData(g_fd, send_buff, sizeof(send_buff));
//                    deserialize_uart_package(buff);
//                    deserialize_cloud_package("leok");

                } else {
                    // printf("funcname:%s line:%d recv_len: %d\n",  __func__, __LINE__, recv_len);
                }
            }
        }
    }
}

/**
*@breif     main()
*/
int main(int argc, char **argv)
{

#if 1
    int fd;
    char *dev ="/dev/ttyUSB0";

    fd = openDevice(dev);
    if (fd > 0)
        setBaudRate(fd, 115200);
    else {
        printf("Can't Open Serial Port!\n");
        exit(0);
    }

    printf("openDevice fd: %d\n", fd);
    // 8位数据，非两位的停止位，不使用奇偶校验 ，不允许输入奇偶校验
    if (setParity(fd, 8, 1, 'n') == false) {
        printf("Set Parity Error\n");
        exit(1);
    }

    g_fd = fd;
#if 0
    // acquire non-blocking exclusive lock
    if (flock(fd, LOCK_EX | LOCK_NB) == -1) {
        printf("Serial port with file descriptor %d i already lockec by another process\n", fd);
    }
#endif
//    char send_buff[12] = {0xFE, 0x09, 0x10, 0x36, 0x61, 0x6E, 0x24, 0x00, 0x4B, 0x12, 0x00, 0xFF};
//    int bytes = writeData(g_fd, send_buff, sizeof(send_buff));

    pthread_t tid;
    // int ret = pthread_create(&tid, NULL, recvDataThread, &fd);
    int ret = pthread_create(&tid, NULL, recvDataThread, NULL);

    struct termios Opt;
    tcgetattr(fd, &Opt);
    printf("Opt.c_cflag: %d\n", Opt.c_cflag);
    printf("Opt.c_iflag: %d\n", Opt.c_iflag);

#endif


#if 0
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

//    closeDevice(fd);

    return 0;
}
