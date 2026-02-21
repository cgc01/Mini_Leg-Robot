#ifndef __UDPCLIENT_H
#define __UDPCLIENT_H

#include <Arduino.h>
#include "MyWiFi.h"
#include "Reg.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "Debug.h"

#define UDP_SERVER_IP       "192.168.114.162"
#define UDP_SERVER_PORT     5000

void UdpClient_Init(void);
void UdpClient_Process_Proc(void);

#endif
