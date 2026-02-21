#ifndef __SERVO_H
#define __SERVO_H

#include <Arduino.h>
#include "esp32/rom/ets_sys.h"
#include "Usart.h"
#include "string.h"
#include "math.h"
#include "LQR.h"

#define SERVO_NUM           2       // 舵机总数
#define SERVO_LEFT          1       // 左舵机
#define SERVO_RIGHT         2       // 右舵机

#define INST_SYNC_WRITE     0x83
#define SMS_STS_ACC         0x29

// 舵机位置限制参数
#define LEG_HEIGHT_BASE 	20      // 腿部默认高度基准值，值越小，机身越高， 实测范围 -10（最高），52（最低）
#define SERVO_LEFT_MIN  	(2048 + 60)     // 左舵机最低位置，大腿与小腿碰到一起
#define SERVO_RIGHT_MIN  	(2048 - 60)     // 右舵机最低位置，大腿与小腿碰到一起
#define SERVO_LEFT_MAX 		(2048 + 12 + 8.4 * (35 + 10))		// 左舵机最高
#define SERVO_RIGHT_MAX     (2048 - 12 - 8.4 * (35 + 10))       // 右舵机最高 

typedef struct Servo_Packet {
    unsigned char ucID;
    short sPosition;
    unsigned short usSpeed;
    unsigned char ucAcc;
} Servo_Packet_t;

float Servo_AngleToHeight(void);
void Servo_SetParma(unsigned char ucID, short sPosition, unsigned short usSpeed, unsigned char ucAcc);
void Servo_Move(void);
void Servo_Init(void);

#endif
