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
#include "list_control.h"

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
    char buff[512] = {0};
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
                    dumpData(buff, recv_len);
                    char send_buff[7] = {0xFC, 0x05, 0x01, 0x02, 0x31, 0x32, 0x33};
                    int bytes = writeData(g_fd, send_buff, 7);
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

#if 0
    int fd;
    char *dev ="/dev/ttyUSB0";
    char send_buff[7] = {0xFC, 0x05, 0x01, 0x02, 0x31, 0x32, 0x33};

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
    int bytes = writeData(fd, send_buff, 7);

    pthread_t tid;
    // int ret = pthread_create(&tid, NULL, recvDataThread, &fd);
    int ret = pthread_create(&tid, NULL, recvDataThread, NULL);

    struct termios Opt;
    tcgetattr(fd, &Opt);
    printf("Opt.c_cflag: %d\n", Opt.c_cflag);
    printf("Opt.c_iflag: %d\n", Opt.c_iflag);

#endif

    TerminalInfo ter_info = {0};
    uint8_t id[4] = {0x01, 0x02, 0x03, 0x04};
//    snprintf(ter_info.id, sizeof(ter_info.id), "%s", id);
    memcpy(ter_info.id, id, sizeof(ter_info.id));
    printf("ter_info.id: \n");
    dumpData(ter_info.id, 4);

    uint8_t pid[2] = {0x05, 0x06};
//    snprintf(ter_info.pid, sizeof(ter_info.pid), "%s", pid);
    memcpy(ter_info.pid, pid, sizeof(ter_info.pid));
    printf("ter_info.pid: \n");
    dumpData(ter_info.pid, 2);

    uint8_t vid[2] = {0x07, 0x08};
//    snprintf(ter_info.vid, sizeof(ter_info.vid), "%s", vid);
    memcpy(ter_info.vid, vid, sizeof(ter_info.vid));
    printf("ter_info.vid: \n");
    dumpData(ter_info.vid, 2);

    uint8_t mac[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
//    snprintf(ter_info.mac, sizeof(ter_info.mac), "%s", mac);
    memcpy(ter_info.mac, mac, sizeof(ter_info.mac));
    printf("ter_info.mac: \n");
    dumpData(ter_info.mac, 8);

    int insert_ret =  insert_item_to_list((void *)&ter_info);

    uint8_t ter_label[17] = {0};
    snprintf(ter_label, sizeof(ter_label), "%02x%02x%02x%02x%02x%02x%02x%02x",
             id[0],
             id[1],
             id[2],
             id[3],
             pid[0],
             pid[1],
             vid[0],
             vid[1]);
//    memcpy(ter_label, id, 4);
//    memcpy(&ter_label[4], pid, 2);
//    memcpy(&ter_label[6], vid, 2);
    printf("ter_label: \n");
    dumpData(ter_label, sizeof(ter_label));
    uint8_t * mac_result = get_mac_from_list(ter_label);
    printf("mac_result: %s\n", mac_result);
    if(mac_result) {
        free(mac_result);
        mac_result = NULL;
    }


    while (getchar() != 'q') {
        usleep(10000);
    }

//    closeDevice(fd);

    return 0;
}
