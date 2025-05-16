#pragma once

#include <esp_err.h>
#include <esp_wifi.h>

#include "esp32center.h"

typedef struct {
    float rockLX;
    float rockLY;
    float rockRX;
    float rockRY;
    uint8_t keyJ1 : 1;
    uint8_t keyJ2 : 1;
    uint8_t keyL1 : 1;
    uint8_t keyL2 : 1;
    uint8_t keyL3 : 1;
    uint8_t keyL4 : 1;
    uint8_t keyR1 : 1;
    uint8_t keyR2 : 1;
    uint8_t keyR3 : 1;
    uint8_t keyR4 : 1;
}CenterJoystickData_t;

typedef struct {
    const char *ssid;
    const char *pswd;
    wifi_mode_t WIFI_MODE_x;
}CenterJoystickConf_t;

// 外设控制接口
esp_err_t BoardCenterJoystickCtrl_Init(const CenterJoystickConf_t *confExt,const CenterConf_t *confCen);
esp_err_t BoardCenterJoystickCtrl_GetData(CenterJoystickData_t *data);

// 实现的功能1：作为ESP-NOW-Remote(Nr)协议遥控主机
#define NR_CONF_EXT {.ssid="esp32-wifi6",.pswd="12345678",.WIFI_MODE_x=WIFI_MODE_AP,}

esp_err_t BoardFlighterApp_NrInit(uint8_t localPort,uint8_t remotePort);
esp_err_t BoardFlighterApp_NrMain(void);