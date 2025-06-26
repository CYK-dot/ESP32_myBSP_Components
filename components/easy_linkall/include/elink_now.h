/**
 * @file easy_nowlink.h
 * @author CYK-Dot
 * @brief 基于MavLink的通用多信道通信--ESPNOW实现
 * @version 0.1
 * @date 2025-06-26
 *
 * @copyright Copyright (c) 2025
 */
#pragma once

/* 头文件引入 -----------------------------------------------------------------------*/

#include <esp_err.h>

#include <mavlink_types.h>

#include "elink_ping.h"

/* 配置宏定义 -----------------------------------------------------------------------*/

#define ELINK_NOW_REMOTE_PMK   "LMK_FILGHT12345" ///< esp-now配对码

/* 导出宏定义 -----------------------------------------------------------------------*/

/* 导出类型定义 ---------------------------------------------------------------------*/

typedef struct {
    mavlink_channel_t ch_id;   ///< 同一组件有多个Mavlink信道，例如wifi、uart等，因此利用信道ID区分彼此
    bool isHostAP;             ///< 是否作为AP模式启动
    bool enablePing;           ///< 是否在该信道上启用ping功能
}enowlink_conf_t;

typedef void (*enowlink_rc_callback_t)(const uint8_t peer[6], const mavlink_rc_channels_scaled_t* rc);
typedef void (*enowlink_text_callback_t)(const uint8_t peer[6], const mavlink_statustext_t* msg);

/* C++兼容 --------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

/* 导出函数声明 ----------------------------------------------------------------------*/

// 初始化
esp_err_t elink_now_init(const enowlink_conf_t* conf);
esp_err_t elink_now_init_example_host(void);
esp_err_t elink_now_init_example_client(void);

// ping其他设备
elink_ping_handle_t elink_now_ping_start(uint8_t system_id,uint8_t component_id,uint16_t timeout);
esp_err_t elink_now_ping_promise(elink_ping_handle_t handle);

// 发送RC数据包

// 发送statustext数据包


/* C++兼容 --------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif
