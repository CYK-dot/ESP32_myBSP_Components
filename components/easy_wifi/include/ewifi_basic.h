/**
 * @file ewifi_basic.h
 * @author CYK-Dot
 * @brief 基本wifi操作函数
 * @version 0.1
 * @date 2025-06-23
 *
 * @copyright Copyright (c) 2025
 */
#pragma once

/* 头文件引入 -----------------------------------------------------------------------*/
#include "prv_ewifi.h"

/* 配置宏定义 -----------------------------------------------------------------------*/

/* 导出宏定义 -----------------------------------------------------------------------*/

/* 导出类型定义 ---------------------------------------------------------------------*/

/* C++兼容 --------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

/* 导出函数声明 ----------------------------------------------------------------------*/

// 配置的读取与写入
void ewifi_basic_print_conf(ewifi_conf_t *conf);
esp_err_t ewifi_basic_get_conf_from_default_ap(ewifi_conf_t *conf);
esp_err_t ewifi_basic_get_conf_from_default_sta(ewifi_conf_t *conf);

esp_err_t ewifi_basic_get_conf_from_nvs(ewifi_conf_t *conf,bool isOverride);
esp_err_t ewifi_basic_update_conf_to_nvs(ewifi_conf_t *conf,bool isOverride);

// 配置的局部特化修改(如果有新的配置,就新写一对函数,实现可拓展性)
void ewifi_basic_set_ap_peri_ssid_password(ewifi_conf_t *conf,const char *ssid,const char *password);
void ewifi_basic_set_sta_peri_ssid_password(ewifi_conf_t *conf,const char *ssid,const char *password);

void ewifi_basic_set_ap_ptl_lr(ewifi_conf_t *conf);
void ewifi_basic_set_sta_ptl_lr(ewifi_conf_t *conf);

// 配置下发到wifi
esp_err_t ewifi_basic_init(ewifi_conf_t *conf);
esp_err_t ewifi_wait_connection(wifi_mode_t mode);

/* C++兼容 --------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif