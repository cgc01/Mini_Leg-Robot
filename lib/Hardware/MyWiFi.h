#ifndef __MYWIFI_H
#define __MYWIFI_H

#include <Arduino.h>
#include <WiFi.h>

typedef enum WiFi_Mode {
    AP,
    STA
} WiFi_Mode_t;

unsigned char MyWiFi_CheckState(void);
void MyWiFi_ReConnect(void);
void MyWiFi_Init(WiFi_Mode_t mode);

#endif
