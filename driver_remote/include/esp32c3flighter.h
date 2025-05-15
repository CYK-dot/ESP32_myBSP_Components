#pragma once

#include <esp_err.h>

// 外设控制接口
esp_err_t BoardFlighterCtrl_Init(const char *ssid,const char *pswd,MyProtocolWifi_t WIFI_PTL_80211_xx);
esp_err_t BoardFlighterCtrl_Bdc(float duty,uint8_t bdcId);
esp_err_t BoardFlighterCtrl_Servo(float angle,uint8_t servoId);

// 实现的功能1：作为ESP-NOW-Remote(nr)协议遥控从机
esp_err_t BoardFlighterApp_NrInit(uint8_t localPort,uint8_t remotePort);
esp_err_t BoardFlighterApp_NrMain(void);

