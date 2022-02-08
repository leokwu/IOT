//
// Created by leok on 2022/1/18.
//

#ifndef PROJECT_SERIAL_CONTROL_H
#define PROJECT_SERIAL_CONTROL_H
#ifdef __cplusplus
extern "C" {
#endif

void setBaudRate(int fd, int speed);
int setParity(int fd, int databits, int stopbits, int parity);
int openDevice(char *Dev);
void closeDevice(int fd);
int writeData(int fd, const char *buf, int len);
int readData(int fd, char *buf, int len);


#ifdef __cplusplus
}
#endif


#endif //PROJECT_SERIAL_CONTROL_H
