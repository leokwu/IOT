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
#include "mqtt_async_control.h"



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

void *mqttThread(void *arg)
{
    int ret = 0;

    while(1) {
        ret = mqttMainProcess();
        usleep(10000L);
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

    pthread_t mqtt_tid;
    ret = pthread_create(&mqtt_tid, NULL, mqttThread, NULL);


    while (getchar() != 'q') {
        usleep(10000);
    }

    serialClose();

    return 0;
}
