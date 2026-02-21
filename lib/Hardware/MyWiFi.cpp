#include "MyWiFi.h"

// 配置 AP（热点）模式相关参数
static const char *AP_SSID = "WLROBOT-TD";
static const char *AP_PSW = "wlrobot123";
static IPAddress AP_IP(192, 168, 1, 11);
static IPAddress AP_GATEWAY(192, 168, 1, 11);
static IPAddress AP_SUBNET(255, 255, 255, 0);

// 配置 STA 模式相关参数
static const char *STA_SSID = "test";
static const char *STA_PWD = "123456789";

/**
 * @brief 初始化 WiFi
 */
void MyWiFi_Init(WiFi_Mode_t Mode)
{
    switch(Mode)
    {
        case AP:
            WiFi.mode(WIFI_AP);
            WiFi.softAPConfig(AP_IP, AP_GATEWAY, AP_SUBNET);
            WiFi.softAP(AP_SSID, AP_PSW);
            break;

        case STA:
            WiFi.mode(WIFI_STA);
            WiFi.setAutoReconnect(true);        // 自动重连
            WiFi.begin(STA_SSID, STA_PWD);
            while(WiFi.status() != WL_CONNECTED){delay(500);}
            break;
    }
}
