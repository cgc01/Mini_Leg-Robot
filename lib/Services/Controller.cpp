#include "Controller.h"

static JumpState_t JumpState = Jump_Idle;       //跳跃状态机

/**
 * @brief 小车转向控制
 *
 * 通过偏航角 + 偏航角速度的 PID 控制叠加操纵杆输入完成小车转向。
 * 内部包含角度环与角速度环，并实现偏航角累计修正（防止角度跳变）。
 *
 * 转向条件：
 *  - 若 JumpState == Jump_Fly，则禁止任何转向输出（避免跳跃时扰动姿态）。
 *
 * @return 转向控制量（正/负表示左右转）
 */
static float Controller_Steering(void)
{
    if(JumpState != Jump_Idle){return 0.0f;}        // 跳跃时不允许叠加转向

    static float fYawAngleTotal = 0;
    static float fYawAngleLast = 0;
    float fYawAngle, fYawGyro, fYawAngle_1, fYawAngle_2, fYawAddupAangle;

    fYawAngle = (float)sReg[AngleZ] * 1.0e-3f / PI * 180.0f;     // 偏航角度
    fYawGyro = (float)sReg[GyroZ] * 1.0e-3f / PI * 180.0f;        // 偏航角速度，用于纠正小车前后走直线时的角度偏差

    if(fYawAngle > fYawAngleLast)
    {
        fYawAngle_1 = fYawAngle - fYawAngleLast;
        fYawAngle_2 = fYawAngle - fYawAngleLast - 2.0f * PI;
    }
    else
    {
        fYawAngle_1 = fYawAngle - fYawAngleLast;
        fYawAngle_2 = fYawAngle - fYawAngleLast + 2.0f * PI;
    }

    if(fabsf(fYawAngle_1) > fabsf(fYawAngle_2)){fYawAddupAangle = fYawAngle_2;}
    else{fYawAddupAangle = fYawAngle_1;}

    fYawAngleTotal = fYawAngleTotal + fYawAddupAangle;
    fYawAngleLast = fYawAngle;
    fYawAngleTotal += (float)sReg[JoyX] * 1.0e-2f * 1e-1f * 0.02f;        // 0.02，转向灵敏度

    fYawAngle = fYawAngle / 180.0f * PI;
    fYawGyro = fYawGyro / 180.0f * PI;

    return PID_YawAngle(fYawAngleTotal) + PID_YawGyro(fYawGyro);
    // return PID_YawAngle((float)sReg[JoyX] / 100.0f / 10.0f * 0.2f);
}

/**
 * @brief 跳跃动作控制状态机
 *
 * 状态说明：
 *  - Jump_Idle     ：等待跳跃指令
 *  - Jump_Prepare  ：舵机收缩准备
 *  - Jump_Push     ：舵机快速伸展推动地面
 *  - Jump_Fly      ：腾空状态（通过加速度判断落地）
 *  - Jump_Landing  ：落地缓冲，防止立刻重新触发
 *
 * 每次进入某状态都设定舵机位置，使用定时递增 usJumpTimer 控制时序。
 */
static void Controller_Jump(unsigned char ucLoopTime)
{
    static unsigned short usJumpTimer = 0;
    switch(JumpState)
    {
        case Jump_Idle:
            if(sReg[RobJump] == 1)
            {
                usJumpTimer = 0;
                JumpState = Jump_Prepare;
            }
            break;
        
        case Jump_Prepare:
            Servo_SetParma(SERVO_LEFT, SERVO_LEFT_MIN - 40, 450, 250);
            Servo_SetParma(SERVO_RIGHT, SERVO_RIGHT_MIN + 40, 450, 250);
            Servo_Move();

            JumpState = Jump_Push;
            break;

        case Jump_Push:
            if((usJumpTimer += ucLoopTime) >= 200 && (usJumpTimer = 0, 1))
            {
                Servo_SetParma(SERVO_LEFT, SERVO_LEFT_MAX + 20, 0, 0);
                Servo_SetParma(SERVO_RIGHT, SERVO_RIGHT_MAX - 40, 0, 0);
                Servo_Move();

                JumpState = Jump_Fly;
            }
            break;
        
        case Jump_Fly:
            if((usJumpTimer += ucLoopTime) >= 100 && (usJumpTimer = 0, 1))
            {
                Servo_SetParma(SERVO_LEFT, SERVO_LEFT_MIN, 0, 0);
                Servo_SetParma(SERVO_RIGHT, SERVO_RIGHT_MIN, 0, 0);
                Servo_Move();

                JumpState = Jump_Landing;
            }
            break;

        case Jump_Landing:
            if(((usJumpTimer += ucLoopTime) >= 700 && (usJumpTimer = 0, 1)) || fabsf((float)sReg[AccZ] * 1.0e-3f) >= 1.3f)
            {
                JumpState = Jump_Landed;
            }
            break;

        case Jump_Landed:
            if((usJumpTimer += ucLoopTime) >= 50 && (usJumpTimer = 0, 1))
            {
                JumpState = Jump_Idle;
            }
    }
}

/**
 * @brief 机器人腿部控制（含跳跃、姿态补偿）
 *
 * 功能：
 *  - 调用跳跃状态机
 *  - 在正常状态（Jump_Idle）下执行左右腿高度调整
 *  - 根据 roll 角度自动调整左右腿高度差，维持机身水平
 *  - 执行舵机限位保护与同步写入
 */
static void Controller_Leg(void)
{
    Controller_Jump(2);

    if(JumpState == Jump_Idle)
    {
        float LegPositionAdd = PID_RollAngle(LPF_Roll((float)sReg[AngleX] * 1.0e-3f + 2.0f));

        short sRollOffset = sReg[TiltAngle] * 1.0e-2f;
        short sLeftHight, sRightHight;

        //如果控制了roll角度，则调整左右脚的高度
        float fAngleRad = fabsf(sRollOffset) / 180.0f * PI;        //将角度转换为弧度
        float fHightAdjust = 30 * sin(fAngleRad);        //小车宽度的一半是30毫米，抬高的高度是30毫米*sin(弧度)
        float fHightOffset = 18;        //水平高度需要加上调整高度，否则向下倾斜无法向下了

        if(sRollOffset > 10)
        {
            sLeftHight = (unsigned short)sReg[RobHeight] * 1.0e-2f + fHightOffset + fHightAdjust;
            sRightHight = (unsigned short)sReg[RobHeight] * 1.0e-2f + fHightOffset - fHightAdjust;
            LegPositionAdd = 0;     //不使用roll角度，pid计算
        }
        else if(sRollOffset < -10)
        {
            sLeftHight = (unsigned short)sReg[RobHeight] * 1.0e-2f + fHightOffset - fHightAdjust;
            sRightHight = (unsigned short)sReg[RobHeight] * 1.0e-2f + fHightOffset + fHightAdjust;
            LegPositionAdd = 0;     //不使用roll角度，pid计算
        }
        else        //未控制roll角度
        {
            sLeftHight = (unsigned short)sReg[RobHeight] * 1.0e-2f;
            sRightHight = (unsigned short)sReg[RobHeight] * 1.0e-2f;
        }

        //2048是大腿紧碰小腿时舵机的位置，12是微调偏移量，8.4是系数
        //LEG_HEIGHT_BASE值越小，机身越高，实测范围 0（最高），52（最低）
        short sPosition[2];
        sPosition[0] = 2048 + 12 + 8.4f * (sLeftHight - LEG_HEIGHT_BASE) + LegPositionAdd;
        sPosition[1] = 2048 - 12 - 8.4f * (sRightHight - LEG_HEIGHT_BASE) - LegPositionAdd;

        // 限制舵机位置范围
        if (sPosition[0] < SERVO_LEFT_MIN)
            sPosition[0] = SERVO_LEFT_MIN;
        else if (sPosition[0] > SERVO_LEFT_MAX)
            sPosition[0] = SERVO_LEFT_MAX;

        if (sPosition[1] > SERVO_RIGHT_MIN)
            sPosition[1] = SERVO_RIGHT_MIN;
        else if (sPosition[1] < SERVO_RIGHT_MAX)
            sPosition[1] = SERVO_RIGHT_MAX;
        
        Servo_SetParma(SERVO_LEFT, sPosition[0], 500, 150);
        Servo_SetParma(SERVO_RIGHT, sPosition[1], 500, 150);
        Servo_Move();
    }
}

/**
 * @brief 小车整体平衡控制
 */
static float Controller_Balance(void)
{
    // 获取传感器数据
    float fLQR_Distance = -(Motor_Left.shaft_angle + Motor_Right.shaft_angle) * LQR_Car.fR / 2.0f;     // 平动位移
    float fLQR_Speed = -(Motor_Left.shaft_velocity + Motor_Right.shaft_velocity) * LQR_Car.fR / 2.0f;      // 平动速度
    float fLQR_Angle = (float)sReg[AngleY] * 1.0e-3f;        // Pitch 角度
    float fLQR_Gyro = (float)sReg[GyroY] * 1.0e-3f;      // Pitch 角速度

    // 初次运行时取平动偏置
    if(!sReg[MotorInitFlag])
    {
        static double dMeanPosZeroOffset = 0;
        static unsigned short usMeanPosZeroOffsetCount = 0; 
        if(fabsf(fLQR_Speed) > 0.03f){return 0.0f;}     // 基本静止才取偏置
        dMeanPosZeroOffset += fLQR_Distance;
        usMeanPosZeroOffsetCount++;
        if(usMeanPosZeroOffsetCount < 1000){return 0.0f;}
        sReg[MotorInitFlag] = 1;
        LQR_Car.fMechWheelPosZeroOffset = (float)(dMeanPosZeroOffset / usMeanPosZeroOffsetCount);
    }

    // 被快速推动或离地时高速旋转或跳跃状态时重置位移零点
    if(fabsf(fLQR_Speed) > 0.5f || JumpState != Jump_Idle)
    {
        LQR_Car.fMechWheelPosZeroOffset = fLQR_Distance;
    }

    // 速度积分驱动位移
    float fJoyY = (float)sReg[JoyY] * 1.0e-4f;
    float fTargetSpeed = fJoyY * LQR_Car.fMaxSpeed;
    static unsigned long long ullSpeedIntegralTime[2] = {0};       // [0] 为最新时间戳，[1] 为上一周期时间戳
    ullSpeedIntegralTime[1] = ullSpeedIntegralTime[0];
    ullSpeedIntegralTime[0] = esp_timer_get_time();
    LQR_Car.fMechWheelPosZeroOffset += fTargetSpeed * (float)(ullSpeedIntegralTime[0] - ullSpeedIntegralTime[1]) * 1e-6f;

    // 根据重心高度更新 K 矩阵
    static float fHeight[2] = {0};        // fHeight[0] 为当前高度，fHeight[1] 为上一次高度
    fHeight[1] = fHeight[0];
    fHeight[0] = Servo_AngleToHeight();
    if(fHeight[0] != fHeight[1]){LQR_UpdateDynamicHeight(fHeight[0]);}

    // 反馈力矩计算
    float fAngleControl = (fLQR_Angle - LQR_Car.fMechAngleZeroOffset) * fLQR_K[0];
    float fGyroControl  = fLQR_Gyro * fLQR_K[1];
    float fDistanceControl = (fLQR_Distance - LQR_Car.fMechWheelPosZeroOffset) * fLQR_K[2];        // 平动位移控制量
    float fSpeedControl = fLQR_Speed * fLQR_K[3];      // 目标速度

    float fLQR_u = fAngleControl + fGyroControl + fDistanceControl + fSpeedControl;

    // 小车没有控制的时候自稳定状态
    if(!sReg[JoyX] && !sReg[JoyY] && fabsf(fLQR_u) < 0.08f && fabsf(fDistanceControl) < 0.15f && JumpState == Jump_Idle && !sReg[MotorStopFlag] && fabsf(fLQR_Angle) < 0.174532925199433f)
    {
        // fLQR_u = PID_LQR_u(fLQR_u);     // 小转矩非线性补偿
        LQR_Car.fMechAngleZeroOffset -= PID_AngleZeroPoint(fLQR_Distance - LQR_Car.fMechWheelPosZeroOffset);
    }
    else{PID_LQR_u.error_prev = 0.0f;}      // 清空小转矩非线性补偿的 PID 积分

    return fLQR_u;
}

/**
 * @brief 初始化控制系统
 */
void Controller_Init(void)
{
    Servo_Init();
    Motor_Init();
}

/**
 * @brief 主控制进程函数
 */
void Controller_Proc(void)
{
    Controller_Leg();
    float fLQR_u = Controller_Balance();
    float fSteeringAngle = Controller_Steering();

    // 跌倒保护
    float fAngleYTemp = (float)sReg[AngleY] * 1.0e-3f / PI * 180.0f;
    if(fAngleYTemp <= -30.0f || fAngleYTemp >= 45.0f){sReg[MotorStopFlag] = 1;}
    else{sReg[MotorStopFlag] = 0;}

    // 应用目标力矩
    if(!sReg[MotorStopFlag])
    {
        Motor_Left.move(fLQR_u - fSteeringAngle);
        Motor_Right.move(fLQR_u + fSteeringAngle);
    }
}
