#include "UdpClient.h"

static int iUdpSocket = -1;
static sockaddr_in ServerAddr;      // 目标服务器地址
static sockaddr_in LocalAddr;       // 本地绑定地址

/**
 * @brief 创建 UDP socket
 */
static unsigned char UdpClient_CreateSocket(void)
{
    iUdpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(iUdpSocket < 0){return 1;}

    // 设置服务器地址
    memset(&ServerAddr, 0, sizeof(ServerAddr));
    ServerAddr.sin_family = AF_INET;
    ServerAddr.sin_port = htons(UDP_SERVER_PORT);
    ServerAddr.sin_addr.s_addr = inet_addr(UDP_SERVER_IP);

    // 绑定本地地址和端口
    memset(&LocalAddr, 0, sizeof(LocalAddr));
    LocalAddr.sin_family = AF_INET;
    LocalAddr.sin_port = htons(UDP_SERVER_PORT);        // 使用相同端口进行
    LocalAddr.sin_addr.s_addr = INADDR_ANY;
    if(bind(iUdpSocket, (sockaddr *)&LocalAddr, sizeof(LocalAddr)) < 0)
    {
        close(iUdpSocket);
        iUdpSocket = -1;
        return 1;
    }

    return 0;
}

/**
 * @brief 关闭 UDP socket
 */
static void UdpClient_CloseSocket(void)
{
    if(iUdpSocket >= 0)
    {
        close(iUdpSocket);
        iUdpSocket = -1;
    }
}

/**
 * @brief 解析收到的控制数据包
 * 
 * @param ucData 指向接收到的6字节数据
 * 
 * 数据协议（固定 6 字节）：
 *  Byte0: 0xFF
 *  Byte1: 0xAA
 *  Byte2: 控制 ID
 *  Byte3: 数据低 8 位
 *  Byte4: 数据高 8 位
 *  Byte5: 校验和（Byte2 + Byte3 + Byte4）
 */
static void UdpClient_ParseRxData(unsigned char *ucData)
{
    unsigned char ucSum = ucData[2] + ucData[3] + ucData[4];
    if(ucData[0] != 0xFF || ucData[1] != 0xAA || ucSum != ucData[5]){return;}       // 检查包头与校验位

    short sData = (short)(((unsigned short)ucData[4] << 8) | ucData[3]);       // 注意：上位机发送时放大了 100 倍

    switch(ucData[2])
    {
        case 0x01:      // 前后
            sReg[JoyY] = sData;
            break;

        case 0x02:      // 左右
            sReg[JoyX] = sData;
            break;

        case 0x03:      // 倾角
            sReg[TiltAngle] = sData;
            break;

        case 0x04:      // 高度
            sReg[RobHeight] = sData;
            break;

        case 0x10:      // 跳跃
            if(sData == 100){sReg[RobJump] = 1;}
            else{sReg[RobJump] = 0;}
            break;

        case 0xFA:
            if(sData){sReg[SendFlag] = 1;}
            else{sReg[SendFlag] = 0;}
            break;
    }
}

/**
 * @brief 构建发送的数据包
 * 
 * @param uiLen 输出，数据包长度
 * @return 指向要发送的数据缓冲区的地址
 */
static unsigned char *UdpClient_ParseTxData(unsigned int *uiLen)
{
    static unsigned char ucTxData[256];
    unsigned char ucIndex = 0;

    ucTxData[ucIndex++] = 0xFF;
    ucTxData[ucIndex++] = 0xAA;

    ucIndex++;      // 包长，先占位

    // // Data 1
    // ucTxData[ucIndex++] = 0x01;
    // memcpy(&ucTxData[ucIndex], &sReg[LQR_Distance], sizeof(float));
    // ucIndex += sizeof(float);

    // // Data 2
    // ucTxData[ucIndex++] = 0x02;
    // memcpy(&ucTxData[ucIndex], &sReg[LQR_Speed], sizeof(float));
    // ucIndex += sizeof(float);

    // // Data 3
    // ucTxData[ucIndex++] = 0x03;
    // memcpy(&ucTxData[ucIndex], &sReg[Height], sizeof(float));
    // ucIndex += sizeof(float);

    // // Data 4
    // ucTxData[ucIndex++] = 0x04;
    // memcpy(&ucTxData[ucIndex], &sReg[LQR_u], sizeof(float));
    // ucIndex += sizeof(float);

    // // Data 5
    // ucTxData[ucIndex++] = 0x05;
    // memcpy(&ucTxData[ucIndex], &sReg[ZeroPoint], sizeof(float));
    // ucIndex += sizeof(float);

    unsigned char ucSum = 0;
    for(unsigned char i = 0; i < ucIndex - 2; i++)
    {
        ucSum += ucTxData[i + 2];
    }
    ucTxData[ucIndex++] = ucSum;

    ucTxData[2] = ucIndex;      // 整包长度
    *uiLen = ucIndex;

    return ucTxData;
}

/**
 * @brief UDP 接收
 */
static void UdpClient_Recv(void)
{
    if(iUdpSocket < 0){return;}

    sockaddr_in FromAddr;
    socklen_t FromLen = sizeof(FromAddr);

    unsigned char ucData[6];
    if(recvfrom(iUdpSocket, ucData, 6, MSG_DONTWAIT, (sockaddr *)&FromAddr, &FromLen) == 6)
    {
        UdpClient_ParseRxData(ucData);      // 处理接收到的数据
    }
}

/**
 * @brief UDP 发送
 */
static void UdpClient_Send(const unsigned char *ucData, unsigned int uiLen)
{
    if(iUdpSocket < 0){return;}
    sendto(iUdpSocket, ucData, uiLen, 0, (sockaddr *)&ServerAddr, sizeof(ServerAddr));
}

/**
 * @brief UDP Client 状态监测
 */
static void UdpClient_StateMonitor(void)
{
    static unsigned int uiLastCheckTick = 0;
    if(millis() - uiLastCheckTick < 1000){return;}      // 每秒检查一次
    uiLastCheckTick = millis();

    static wl_status_t LastWiFiStatus = WL_CONNECTED;       // 能过初始化则定为已连接

    wl_status_t Cur = WiFi.status();
    if(LastWiFiStatus == WL_CONNECTED && Cur != WL_CONNECTED)
    {
        UdpClient_CloseSocket();        // 断线，关闭 socket
    }
    else if(LastWiFiStatus != WL_CONNECTED && Cur == WL_CONNECTED && iUdpSocket < 0)
    {
        UdpClient_CreateSocket();       // 重连成功，重新创建 socket
    }

    LastWiFiStatus = Cur;
}

/**
 * @brief UDP Client 初始化
 */
void UdpClient_Init(void)
{
    MyWiFi_Init(STA);

    // 创建 UDP Socket
    while(UdpClient_CreateSocket()){delay(1000);}
}

/**
 * @brief UDP Client 进程函数
 */
void UdpClient_Process_Proc(void)
{
    UdpClient_StateMonitor();

    // UDP 接收数据
    UdpClient_Recv();

    // UDP 发送数据，一问一答形式
    if(sReg[SendFlag] && (sReg[SendFlag] = 0, 1))
    {
        unsigned int uiTxLen;
        // unsigned char *ucTxData = UdpClient_ParseTxData(&uiTxLen);
        unsigned char *ucTxData = Debug_PackDebugData(&uiTxLen);
        UdpClient_Send(ucTxData, uiTxLen);
    }

    // Socket 断开则重置控制
    if(iUdpSocket < 0)
    {
        sReg[JoyX] = 0;
        sReg[JoyY] = 0;
        sReg[RobJump] = 0;
    }
}
