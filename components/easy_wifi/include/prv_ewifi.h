/**
 * @file easy_wifi.h
 * @author CYK-Dot
 * @brief 简易WiFi管理
 * @version 0.1
 * @date 2025-06-23
 *
 * @copyright Copyright (c) 2025
 */
#pragma once

/* 头文件引入 -----------------------------------------------------------------------*/

#include <esp_err.h>
#include <esp_wifi.h>
#include <esp_wifi_types_generic.h>

/* 配置宏定义 -----------------------------------------------------------------------*/

// NVS配置
#define EASY_WIFI_NVS_NAMESPACE "my_esp32"
#define EASY_WIFI_NVS_KEY_NAME  "easy_wifi"

// AP/STA通用配置
#define EASY_WIFI_DEFAULT_SSID      "我是ESP32,密码为12345678"
#define EASY_WIFI_DEFAULT_PASSWORD  "12345678"
#define EASY_WIFI_DEFAULT_MODE      WIFI_MODE_AP

// AP配置
#define EASY_WIFI_DEFAULT_AP_SSID_HIDDEN false                  /// 隐藏SSID
#define EASY_WIFI_DEFAULT_AP_AUTH      WIFI_AUTH_WPA2_WPA3_PSK  /// 加密方式
#define EASY_WIFI_DEFAULT_AP_CIPHER    WIFI_CIPHER_TYPE_CCMP    /// 单播加密方式
#define EASY_WIFI_DEFAULT_AP_SAE       WPA3_SAE_PWE_UNSPECIFIED /// 不抗暴力破解
#define EASY_WIFI_DEFAULT_AP_PMF_CAPABLE     true               /// 对wifi管理帧进行保护
#define EASY_WIFI_DEFAULT_AP_PMF_REQUIRE     false              /// 但不强制对wifi管理帧进行保护
#define EASY_WIFI_DEFAULT_AP_TRANSITION      0                  /// 同时允许WPA2+WPA3设备接入

#define EASY_WIFI_DEFAULT_AP_MAX_CONNECTION  4   /// 最大连接数
#define EASY_WIFI_DEFAULT_AP_BEACON_INTERVAL 100 /// 多少毫秒发送一次广播
#define EASY_WIFI_DEFAULT_AP_CSA_INTERVAL    20  /// 等待多少次beacon广播后再切换信道，以等待STA做出响应
#define EASY_WIFI_DEFAULT_AP_DTIM_INTERVAL   3   /// 多少次beacon广播后，发送一次DTIM广播，以唤醒STA

// STA配置
#define EASY_WIFI_DEFAULT_STA_AUTH  WIFI_AUTH_WPA_WPA2_PSK
#define EASY_WIFI_DEFAULT_STA_SAE_PWE   WPA3_SAE_PWE_BOTH /// 不抗暴力破解
#define EASY_WIFI_DEFAULT_STA_SAE_PK_MODE WPA3_SAE_PK_MODE_AUTOMATIC
#define EASY_WIFI_DEFAULT_STA_TRANSITION      0              /// 同时允许WPA2+WPA3接入
#define EASY_WIFI_DEFAULT_STA_PK    false                    /// 不启用SAE-PK认证
#define EASY_WIFI_DEFAULT_STA_PMF_CAPABLE     true           /// 对wifi管理帧进行保护
#define EASY_WIFI_DEFAULT_STA_PMF_REQUIRE     false          /// 但不强制对wifi管理帧进行保护

#define EASY_WIFI_DEFAULT_STA_BTM false /// 关闭AP邻域报告(802.11k)
#define EASY_WIFI_DEFAULT_STA_MBO false /// 关闭2.4G/5G多频段运营(802.11v)
#define EASY_WIFI_DEFAULT_STA_FT  false /// 关闭快速传输(802.11r)
#define EASY_WIFI_DEFAULT_STA_OWE false /// 关闭开放组网加密

#define EASY_WIFI_DEFAULT_STA_SCAN  WIFI_FAST_SCAN
#define EASY_WIFI_DEFAULT_STA_BSSID_SET false  /// 不与指定AP绑定连接
#define EASY_WIFI_DEFAULT_STA_CHANNEL 0        /// 自动选择信道
#define EASY_WIFI_DEFAULT_STA_INTERVAL 0       /// STA休眠后，每隔多少个DTIM广播后，唤醒STA，0代表自动选择
#define EASY_WIFI_DEFAULT_STA_SORT_METHOD WIFI_CONNECT_AP_BY_SIGNAL /// 按信号强度排序
#define EASY_WIFI_DEFAULT_STA_RSSI -70                              /// 信号强度阈值
#define EASY_WIFI_DEFAULT_STA_RETRY 5                               /// 连接失败后，重试次数

#define EASY_WIFI_DEFAULT_STA_HE_DCM false /// 802.11AX独有：启用高密度调制，提升抗干扰
#define EASY_WIFI_DEFAULT_STA_HE_DCM_TX 3  /// 802.11AX独有：发送端调制的阶数，3代表16QAM
#define EASY_WIFI_DEFAULT_STA_HE_DCM_RX 3  /// 802.11AX独有：接收端调制的阶数，3代表16QAM
#define EASY_WIFI_DEFAULT_STA_HE_MCS9 false /// 802.11AX独有：启用MCS9，提升速率(1024QAM)
#define EASY_WIFI_DEFAULT_STA_HE_SU false   /// 802.11AX独有：关闭波束成形

// 频段配置
#define EASY_WIFI_DEFAULT_BANDWIDTH WIFI_BW_HT40
#define EASY_WIFI_DEFAULT_AP_CHANNEL_P 6
#define EASY_WIFI_DEFAULT_AP_CHANNEL_S WIFI_SECOND_CHAN_BELOW

// 协议配置
#define EASY_WIFI_DEFAULT_PTL       WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N

/* 导出宏定义 -----------------------------------------------------------------------*/

/* 导出类型定义 ---------------------------------------------------------------------*/

/**
 * @brief wifi外设的配置
 * 
 */
typedef struct {
    wifi_config_t config;
    wifi_mode_t mode;
}ewifi_conf_peri_t;

/**
 * @brief wifi协议层的配置
 * 
 */
typedef struct {
    uint8_t protocol;
}ewifi_conf_ptl_t;

/**
 * @brief wifi物理层的配置
 * 
 */
typedef struct {
    wifi_bandwidth_t bandwidth;
    uint8_t primary_channel;
    uint8_t secondary_channel;
}ewifi_conf_phy_t;

/**
 * @brief 完整的wifi配置
 * 
 */
typedef struct {
    ewifi_conf_peri_t peri;
    ewifi_conf_ptl_t  ptl;
    ewifi_conf_phy_t  phy;
}ewifi_conf_t;

/* C++兼容 --------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

/* 导出函数声明 ----------------------------------------------------------------------*/

/* C++兼容 --------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif
