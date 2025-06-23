/**
 * @file easy_wifi.h
 * @author CYK-Dot
 * @brief 命令行式WiFi管理
 * @version 0.1
 * @date 2025-06-23
 *
 * @copyright Copyright (c) 2025
 */
#pragma once

/* 头文件引入 -----------------------------------------------------------------------*/

#include <esp_err.h>
#include <esp_wifi.h>

/* 配置宏定义 -----------------------------------------------------------------------*/

#define EASY_WIFI_NVS_NAMESPACE "my_esp32"
#define EASY_WIFI_NVS_KEY_NAME  "easy_wifi"

#define EASY_WIFI_DEFAULT_SSID      "我是ESP32,密码为12345678"
#define EASY_WIFI_DEFAULT_PASSWORD  "12345678"
#define EASY_WIFI_DEFAULT_MODE      WIFI_MODE_AP

/* 导出宏定义 -----------------------------------------------------------------------*/

/* 导出类型定义 ---------------------------------------------------------------------*/

typedef struct {
    char ssid[32];
    char password[64];
    wifi_mode_t mode;
}ewifi_conf_t;

/* C++兼容 --------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

/* 导出函数声明 ----------------------------------------------------------------------*/

//easy_wifi_nvs.c
esp_err_t ewifi_nvs_init(void);
esp_err_t ewifi_nvs_reset(void);
esp_err_t ewifi_nvs_update(const ewifi_conf_t *conf);

/* C++兼容 --------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif
