#ifndef __REG_H
#define __REG_H

#define BAT                 0x00        // 电压放大 1000 倍

// 加速度放大 1000 倍
#define AccX                0x01
#define AccY                0x02
#define AccZ                0x03

// 角速度放大 1000 倍，弧度制
#define GyroX               0x04
#define GyroY               0x05
#define GyroZ               0x06

// 角度放大 1000 倍，弧度制
#define AngleX              0x07        // Roll
#define AngleY              0x08        // Pitch
#define AngleZ              0x09        // Yaw

#define JoyY                0x0A        // 前后
#define JoyX                0x0B        // 左右
#define TiltAngle           0x0C        // 倾角
#define RobHeight           0x0D        // 机器人高度
#define RobJump             0x0E        // 机器人跳跃

#define SendFlag            0x0F        // WebServer 发送数据标志位，置 1 才允许向 Web 发送数据
#define MotorInitFlag       0x10        // 电机初始化标志位
#define MotorStopFlag       0x11        // 停止电机标志位

#define REGSIZE             0x20
#define REGLAST             (REGSIZE - 1)

extern short sReg[REGSIZE];

void Reg_Init(void);

#endif
