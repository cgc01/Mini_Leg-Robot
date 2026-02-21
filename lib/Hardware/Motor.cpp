#include "Motor.h"

BLDCMotor Motor_Left = BLDCMotor(7, 12.3983f, 100.0f);        // 极对数，线电阻，KV 值
BLDCMotor Motor_Right = BLDCMotor(7, 12.5166f, 100.0f);        // 极对数，线电阻，KV 值
static BLDCDriver3PWM Motor_Driver_Left = BLDCDriver3PWM(32, 33, 25, 22);      // 前3个参数是UH, VH, WH引脚，最后一个是使能EN
static BLDCDriver3PWM Motor_Driver_Right = BLDCDriver3PWM(26, 27, 14, 12);

/**
 * @brief 初始化电机
 */
void Motor_Init(void)
{
    Encoder_Init();
    
    // 绑定传感器
    Motor_Left.linkSensor(&Encoder_Left);
    Motor_Right.linkSensor(&Encoder_Right);

    // 绑定驱动器
    Motor_Left.linkDriver(&Motor_Driver_Left);
    Motor_Right.linkDriver(&Motor_Driver_Right);

    // PWM 调制方式
    Motor_Left.foc_modulation = FOCModulationType::SpaceVectorPWM;
    Motor_Left.foc_modulation = FOCModulationType::SpaceVectorPWM;

    // 驱动器配置
    Motor_Driver_Left.voltage_power_supply = 8.0f;
    Motor_Driver_Right.voltage_power_supply = 8.0f;
    Motor_Driver_Left.init();
    Motor_Driver_Right.init();

    // 电机配置
    Motor_Left.voltage_sensor_align = 6.0f;
    Motor_Right.voltage_sensor_align = 6.0f;
    Motor_Left.controller = MotionControlType::torque;      // 运动控制器类型为扭矩模式
    Motor_Right.controller = MotionControlType::torque;
    Motor_Left.torque_controller = TorqueControlType::voltage;      // 扭矩控制器类型为电压模式
    Motor_Right.torque_controller = TorqueControlType::voltage;

    // 初始化电机
    Motor_Left.init();
    Motor_Left.initFOC();
    Motor_Right.init();
    Motor_Right.initFOC();
}

/**
 * @brief 电机更新进程函数
 */
void Motor_Update_Proc(void)
{
    //迭代计算FOC相电压
    if(!sReg[MotorStopFlag] || !sReg[MotorInitFlag])
    {
        Motor_Left.loopFOC();
        Motor_Right.loopFOC();
    }
}
