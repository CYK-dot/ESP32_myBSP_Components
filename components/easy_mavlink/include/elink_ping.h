/**
 * @file elink_ping.h
 * @author CYK-Dot
 * @brief 基于FreeRTOS的MAVLink-PING服务
 * @version 0.1
 * @date 2025-06-26
 *
 * @copyright Copyright (c) 2025
 */
#pragma once

/* 头文件引入 -----------------------------------------------------------------------*/

#include <stdint.h>

#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>

#include <esp_comm/mavlink.h>
#include <mavlink_helpers.h>

/* 配置宏定义 -----------------------------------------------------------------------*/

#define ELINK_PING_GET_TIMSTAMP xTaskGetTickCount

/* 导出宏定义 -----------------------------------------------------------------------*/

/* 导出类型定义 ---------------------------------------------------------------------*/

/// PING服务句柄
typedef void* elink_ping_handle_t;

/// PING代理函数签名
typedef BaseType_t (*elink_ping_tx_callback_t)(mavlink_channel_t channel,mavlink_message_t *msg,uint16_t pkg_len);

/* C++兼容 --------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

/* 导出函数声明 ----------------------------------------------------------------------*/

void elink_ping_rx_proxy(mavlink_channel_t channel,const mavlink_message_t *msg);
void elink_ping_tx_proxy_set(mavlink_channel_t channel,elink_ping_tx_callback_t cb);

elink_ping_handle_t elink_ping_start(mavlink_channel_t channel,uint8_t sys_id,MAV_COMPONENT com_id,uint16_t timeout_ms);
BaseType_t elink_ping_promise(elink_ping_handle_t handle);

/* C++兼容 --------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif