#include "Servo.h"

static Servo_Packet_t ServoPacket[2];      // [0] 为左舵机，[1] 为右舵机
static Servo_Packet_t ServoPacket_LastState[2];     // 记录上一次的舵机目标参数
static unsigned char ucWriteBuf[128], ucWriteLen = 0;       // 发送缓冲区

/**
 * @brief 读取舵机数据
 */
static void Servo_Read(void)
{
    ;
}

/**
 * @brief 将数据写入舵机发送缓冲区
 * 
 * @param ucData 指向要写入的数据缓冲区
 * @param uiLen 数据长度（字节数）
 * @return 当前缓冲区长度
 */
static unsigned int Servo_Write(unsigned char *ucData, unsigned int uiLen)
{
    while(uiLen--)
    {
        if(ucWriteLen < sizeof(ucWriteBuf))
        {
            ucWriteBuf[ucWriteLen++] = *ucData;
            ucData++;
        }
    }
    return ucWriteLen;
}

/**
 * @brief 接收缓冲区刷新
 * 
 * @note FT舵机总线切换延时，时间大于10us
 */
static void Servo_RFlush(void)
{
    ets_delay_us(15);
}

/**
 * @brief 发送缓冲区数据并清空
 */
static void Servo_WFlush(void)
{
    if(ucWriteLen)
    {
        Usart_WriteBytes(ucWriteBuf, ucWriteLen);
        ucWriteLen = 0;
    }
}

/**
 * @brief 交换 short 数据的高低字节
 * 
 * @param ucDataL 低字节存放指针
 * @param ucDataH 高字节存放指针
 * @param sData 要交换的 short 数据
 */
static void Servo_SwapByte(unsigned char *ucDataL, unsigned char *ucDataH, short sData)
{
    *ucDataH = sData >> 8;
    *ucDataL = sData & 0xFF;
}

/**
 * @brief 将指定舵机当前位置校准为中位
 * 
 * @param ucID 舵机ID
 */
static void Servo_CalibrateMid(unsigned char ucID)
{
    unsigned char ucPacket[6];

    ucPacket[0] = 0xFF;
    ucPacket[1] = 0xFF;
    ucPacket[2] = ucID;
    ucPacket[3] = 0x02;     // 长度，校准无参数时为 2
    ucPacket[4] = 0x0B;     // 中位校准指令

    unsigned char ucSum = ucPacket[2] + ucPacket[3] + ucPacket[4];
    ucPacket[5] = ~ucSum;

    Servo_RFlush();
    Servo_Write(ucPacket, sizeof(ucPacket));
    Servo_WFlush();

    delay(100);
}

/**
 * @brief STS 舵机同步写
 * 
 * @param ucIDNum 要写入的舵机数量
 * @param ucMemAddr 内存表起始地址
 * @param ucNData 每个舵机的数据缓冲区
 * @param ucNLen 每个舵机的数据长度
 */
static void Servo_SyncWrite(unsigned char ucIDNum, unsigned char ucMemAddr, unsigned char *ucNData, unsigned char ucNLen)
{
    unsigned char ucMesLen = (ucNLen + 1) * ucIDNum + 4;

    unsigned char ucPacket[7];
    unsigned char ucIndex = 0;

    ucPacket[ucIndex++] = 0xFF;
    ucPacket[ucIndex++] = 0xFF;
    ucPacket[ucIndex++] = 0xFE;     // 广播

    ucPacket[ucIndex++] = ucMesLen;
    ucPacket[ucIndex++] = INST_SYNC_WRITE;
    ucPacket[ucIndex++] = ucMemAddr;
    ucPacket[ucIndex++] = ucNLen;

    Servo_RFlush();
    Servo_Write(ucPacket, ucIndex);

    unsigned char ucSum = 0xFE + ucMesLen + INST_SYNC_WRITE + ucMemAddr + ucNLen;
    for(unsigned char i = 0; i < ucIDNum; i++)
    {
        Servo_Write(&ServoPacket[i].ucID, 1);
        Servo_Write(&ucNData[i * ucNLen], ucNLen);

        ucSum += ServoPacket[i].ucID;
        for(unsigned char j = 0; j < ucNLen; j++)
        {
            ucSum += ucNData[i * ucNLen + j];
        }
    }
    ucSum = ~ucSum;

    Servo_Write(&ucSum, 1);
    Servo_WFlush();
}

/**
 * @brief STS 舵机设定姿态，构建每舵机数据包并调用同步写
 * 
 * @param ucIDNum 要操作的舵机数量
 */
static void Servo_SyncWritePosEx(unsigned char ucIDNum)
{
    unsigned char ucPacket[ucIDNum * 7];

    Servo_Packet_t ServoPacketTemp;
    for(unsigned char i = 0; i < ucIDNum; i++)
    {
        ServoPacketTemp = ServoPacket[i];

        if(ServoPacket[i].sPosition < 0)
        {
            ServoPacketTemp.sPosition = -ServoPacket[i].sPosition;
            ServoPacketTemp.sPosition |= (1 << 15);
        }

        if(ServoPacket[i].usSpeed){ServoPacketTemp.usSpeed = ServoPacket[i].usSpeed;}
        else{ServoPacketTemp.usSpeed = 0;}

        if(ServoPacket[i].ucAcc){ServoPacketTemp.ucAcc = ServoPacket[i].ucAcc;}
        else{ServoPacketTemp.ucAcc = 0;}

        ucPacket[i * 7 + 0] = ServoPacketTemp.ucAcc;
        Servo_SwapByte(&ucPacket[i * 7 + 1], &ucPacket[i * 7 + 2], ServoPacketTemp.sPosition);
        Servo_SwapByte(&ucPacket[i * 7 + 3], &ucPacket[i * 7 + 4], 0);
        Servo_SwapByte(&ucPacket[i * 7 + 5], &ucPacket[i * 7 + 6], ServoPacketTemp.usSpeed);
    }

    Servo_SyncWrite(ucIDNum, SMS_STS_ACC, ucPacket, 7);
}

/**
 * @brief 将舵机角度变化转换成小车高度
 * 
 * @return 变化后的高度
 */
float Servo_AngleToHeight(void)
{
    float fAngleLeft = fabsf(ServoPacket[0].sPosition - 2048.0f);
    float fAngleRight = fabsf(ServoPacket[1].sPosition - 2048.0f);
    float fMeanAngle = (fAngleLeft + fAngleRight) / 2.0f * 0.088f / 180.0f * PI;        // 平均角度（弧度制）

    static const float fC = (LQR_Car.fBaseHeight * LQR_Car.fBaseHeight + LQR_Car.fThighLength * LQR_Car.fThighLength - LQR_Car.fShankLength * LQR_Car.fShankLength) / (2 * LQR_Car.fBaseHeight * LQR_Car.fThighLength);
    static const float fS = sqrtf(1 - fC * fC);

    float fH = LQR_Car.fThighLength * (fC * cosf(fMeanAngle) + fS * sinf(fMeanAngle)) + sqrtf(LQR_Car.fShankLength * LQR_Car.fShankLength - LQR_Car.fThighLength * LQR_Car.fThighLength * (fS * cosf(fMeanAngle) - fC * sinf(fMeanAngle)) * (fS * cosf(fMeanAngle) - fC * sinf(fMeanAngle)));

    return fH;
}

/**
 * @brief 设置单个舵机参数
 * 
 * @param ucID 舵机索引
 * @param sPosition 目标位置
 * @param usSpeed 速度
 * @param ucAcc 加速度
 */
void Servo_SetParma(unsigned char ucID, short sPosition, unsigned short usSpeed, unsigned char ucAcc)
{
    switch(ucID)
    {
        case SERVO_LEFT:
            memcpy(&ServoPacket_LastState[0], &ServoPacket[0], sizeof(Servo_Packet_t));
            ServoPacket[0] = {
                .ucID = ucID,
                .sPosition = sPosition,
                .usSpeed = usSpeed,
                .ucAcc = ucAcc
            };
            break;
        
        case SERVO_RIGHT:
            memcpy(&ServoPacket_LastState[1], &ServoPacket[1], sizeof(Servo_Packet_t));
            ServoPacket[1] = {
                .ucID = ucID,
                .sPosition = sPosition,
                .usSpeed = usSpeed,
                .ucAcc = ucAcc
            };
            break;
    }
}

/**
 * @brief 移动舵机（执行同步写）
 */
void Servo_Move(void)
{
    int iIsChange = 0;
    for(unsigned char i = 0; i < SERVO_NUM; i++)
    {
        iIsChange = memcmp(&ServoPacket_LastState[i], &ServoPacket[i], sizeof(Servo_Packet_t));
        if(iIsChange){break;}
    }
    if(iIsChange){Servo_SyncWritePosEx(SERVO_NUM);}
}

/**
 * @brief 初始化舵机
 */
void Servo_Init(void)
{
    Usart_Init();

    // 中位校准
    Servo_CalibrateMid(SERVO_LEFT);
    Servo_CalibrateMid(SERVO_RIGHT);

    Servo_SetParma(SERVO_LEFT, SERVO_LEFT_MIN, 400, 30);
    Servo_SetParma(SERVO_RIGHT, SERVO_RIGHT_MIN, 400, 30);
    Servo_Move();
}
