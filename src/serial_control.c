//
// Created by leok on 2022/1/18.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <stdbool.h>

#include "serial_control.h"
 
/**
*@brief  设置串口通信速率
*@param  fd     类型 int  打开串口的文件句柄
*@param  speed  类型 int  串口速度
*@return  void
*/

void setBaudRate(int fd, int speed)
{
    int i;
    int status;
    int speed_arr[] = { B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300,
                        B38400, B19200, B9600, B4800, B2400, B1200, B300, };
    int name_arr[] = { 115200, 38400,  19200,  9600,  4800,  2400,  1200,  300,
                      38400,  19200,  9600, 4800, 2400, 1200,  300, };

    struct termios Opt;
    tcgetattr(fd, &Opt);
    for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++) {
        if (speed == name_arr[i]) {
                tcflush(fd, TCIOFLUSH);//Update the options and do it NOW
                cfsetispeed(&Opt, speed_arr[i]);
                cfsetospeed(&Opt, speed_arr[i]);
                status = tcsetattr(fd, TCSANOW, &Opt);
                if  (status != 0)
                    printf("setBaudRate tcsetattr fd\n");
                return;
        }
        tcflush(fd, TCIOFLUSH);
    }
}

/**       
*@brief   设置串口数据位，停止位和效验位
*@param  fd     类型  int  打开的串口文件句柄*
*@param  databits 类型  int 数据位   取值 为 7 或者8   数据位为7位或8位
*@param  stopbits 类型  int 停止位   取值为 1 或者2*    停止位为1或2位
*@param  parity  类型  int  效验类型 取值为N,E,O,S     N->无奇偶校验，O->奇校验 E->为偶校验，
*@return
*/
int setParity(int fd, int databits, int stopbits, int parity)
{
    struct termios options;
    if ( tcgetattr( fd,&options) != 0) {
        printf("setParity SetupSerial 1\n");
        return false;
    }

    options.c_cflag &= ~CSIZE;
    /*设置数据位数*/
    switch (databits) {
        case 7:
            options.c_cflag |= CS7;
            break;
        case 8:
            options.c_cflag |= CS8;
            break;
        default:
            printf("setParity Unsupported data size\n");
            return false;
    }
    switch (parity) {
        case 'n':
        case 'N':
            options.c_cflag &= ~PARENB;   /* Clear parity enable */
            options.c_iflag &= ~INPCK;     /* Enable parity checking */
            break;
        case 'o':
        case 'O':
            options.c_cflag |= (PARODD | PARENB);  /* 设置为奇效验*/
            options.c_iflag |= INPCK;             /* Disnable parity checking */
            break;
        case 'e':
        case 'E':
            options.c_cflag |= PARENB;     /* Enable parity */
            options.c_cflag &= ~PARODD;   /* 转换为偶效验*/
            options.c_iflag |= INPCK;       /* Disnable parity checking */
            break;
        case 'S':
        case 's':  /*as no parity*/
            options.c_cflag &= ~PARENB;
            options.c_cflag &= ~CSTOPB;
            break;
        default:
            printf("setParity Unsupported parity\n");
            return false;
    }
    /* 设置停止位*/
    switch (stopbits) {
        case 1:
            options.c_cflag &= ~CSTOPB;
            break;
        case 2:
            options.c_cflag |= CSTOPB;
            break;
        default:
            printf("setParity Unsupported stop bits\n");
            return false;
    }
    /* Set input parity option */
    if (parity != 'n') {
        options.c_iflag |= INPCK;
    }

    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    options.c_cc[VTIME] = 150; // 15 seconds
    options.c_cc[VMIN] = 0; // read doesn't block

    tcflush(fd, TCIFLUSH); /* Update the options and do it NOW */
    if (tcsetattr(fd, TCSANOW, &options) != 0) {
        printf("setParity SetupSerial 3\n");
        return false;
    }
    return true;
}

/**
*@breif 打开串口
*@return
*/
int openDevice(char *Dev)
{
    int fd = open( Dev, O_RDWR);         //| O_NOCTTY | O_NDELAY | O_NONBLOCK
    /*设置数据位数*/
    if (-1 == fd) {
        printf("openDevice Can't Open Serial Port :%s\n", Dev);
        return -1;
    } else {
        return fd;
    }
}

/**
*@breif 关闭串口
*@return
*/
void closeDevice(int fd)
{
    if (fd > 0) {
        close(fd);
    }
}


/**
*@brief  往串口写数据
*@param  fd     类型  int  打开的串口文件句柄*
*@param  buf 类型  const char* 需要写的数据
*@param  len 类型  int 需要写的数据长度
*@return bytes 类型 int 写成功的数据长度
*/
int writeData(int fd, const char *buf, int len)
{
    if (fd < 0 || buf == NULL || len == 0) {
        return -1;
    }
    errno = 0;
    int bytes = write(fd, buf, len);
    if (bytes <= 0) {
        printf("write error bytes: %d\n", bytes);
        if (bytes == 0) {
            bytes = -1;
        }
//        tcflush(fd, TCOFLUSH); /* Update the options and do it NOW */
    }
    printf("writeData bytes: %d\n", bytes);
    return bytes;
}

/**
*@brief  读串口数据
*@param  fd     类型  int  打开的串口文件句柄*
*@param  buf 类型  char* 数据存储内存
*@param  len 类型  int 需要读的数据长度
*@return bytes 类型 int 读成功的数据长度
*/
int readData(int fd, char *buf, int len)
{
    if (fd < 0 || buf == NULL || len == 0) {
        return -1;
    }

    int bytes = read(fd, buf, len);
    if (bytes <= 0) {
//        printf("readData read bytes: %d\n", bytes);
        if (bytes == 0) {
            bytes = -1;
        }
//        tcflush(fd, TCIFLUSH); /* Update the options and do it NOW */
    }
//    printf("readData bytes: %d\n", bytes);
    return bytes;
}
