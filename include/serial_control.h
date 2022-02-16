//
// Created by leok on 2022/1/18.
//

#ifndef PROJECT_SERIAL_CONTROL_H
#define PROJECT_SERIAL_CONTROL_H
#ifdef __cplusplus
extern "C" {
#endif


int serialInit();
int serialGetFd();
int serialRead(char *buf, int len);
int serialWrite(const char *buf, int len);
void serialClose();

#ifdef __cplusplus
}
#endif


#endif //PROJECT_SERIAL_CONTROL_H
