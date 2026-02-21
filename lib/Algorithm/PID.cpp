#include "PID.h"

// PID控制器实例
PIDController PID_AngleZeroPoint(0.00001f, 0.0f, 0.0f, 100000.0f, 1.0f);      // 角度零点补偿
PIDController PID_LQR_u(1.0f, 0.12f, 0.0f, 100000.0f, 1.0f);        // 小转矩非线性补偿。注意，积分项与执行周期强相关
PIDController PID_YawAngle(0.02f, 0, 0, 100000, 8);
PIDController PID_YawGyro(0.04f, 0, 0, 100000, 8);
PIDController PID_RollAngle(8.0f, 0, 0, 100000, 450);
