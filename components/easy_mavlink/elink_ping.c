/**
 * @file elink_ping.c
 * @author CYK-Dot
 * @brief 基于FreeRTOS的MAVLink-PING服务
 * @version 0.1
 * @date 2025-06-26
 *
 * @copyright Copyright (c) 2025
 * 
 * @details 使用方式：
 * > 在指定的信道上调用elink_ping_rx_proxy接收、并注册elink_ping_tx_proxy_set。
 * > 使用elink_ping_start创建一个ping请求，并使用elink_ping_promise等待ping结果
 */

/* 头文件引入 -----------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>

#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>
#include <FreeRTOS/timers.h>
#include <FreeRTOS/event_groups.h>
#include <FreeRTOS/semphr.h>

#include <esp_comm/mavlink.h>
#include <esp_comm/mavlink_msg_ping.h>

#include "clib_list.h"

#include "elink_global.h"
#include "elink_ping.h"

/* 私有类型定义 ---------------------------------------------------------------------*/

typedef struct {
    uint8_t sys_id;
    MAV_COMPONENT com_id;
    EventGroupHandle_t event;
    uint32_t seq;
    TimerHandle_t timer;
}elink_ping_handle_internal_t;

/* 私有宏定义 -----------------------------------------------------------------------*/

#define PING_FINISH_EVENT_BIT (1 << 0)
#define PING_TIMEOUT_EVENT_BIT (1 << 1)

/* 全局变量声明 ---------------------------------------------------------------------*/

static uint32_t g_ping_seq = 0;
static list_t * g_ping_list = NULL;
static elink_ping_tx_callback_t g_tx_proxy_list[ELINK_MAVLINK_CHANNEL_MAX] = {NULL};

/* 私有函数声明 ---------------------------------------------------------------------*/

static void elink_timer_callback( TimerHandle_t xTimer );
static void elink_handle_create(elink_ping_handle_internal_t **handle,uint8_t sys_id,MAV_COMPONENT com_id,uint16_t timeout_ms);
static void elink_handle_destroy(elink_ping_handle_internal_t *handle);
static uint16_t elink_ping_get_msg(elink_ping_handle_t handle,mavlink_message_t *msg);

/* 导出函数定义 ---------------------------------------------------------------------*/

/**
 * @brief 启动一个ping服务
 * 
 * @param sys_id 对方系统的ID，暂不支持广播(ID=0)
 * @param com_id 对方组件的ID，暂不支持广播(ID=0)
 * @param timeout_ms 
 * @return elink_ping_handle_t 
 */
elink_ping_handle_t elink_ping_start(mavlink_channel_t channel,uint8_t sys_id,MAV_COMPONENT com_id,uint16_t timeout_ms)
{
    // 自己ping自己是不行的，广播也暂时不支持
    if (sys_id == ELINK_MAVLINK_SYS_ID && com_id == ELINK_MAVLINK_COMP_ID) {
        printf("[E] ELINK_PING$ sys_id and com_id can't be the same as the sender\n");
        return NULL;
    }
    if (sys_id == 0 || com_id == 0) {
        printf("[E] ELINK_PING$ broadcast is not supported now\n");
        return NULL;
    }
    // 创建ping对象
    elink_ping_handle_internal_t *handle = NULL;
    elink_handle_create(&handle, sys_id, com_id, timeout_ms);
    if (handle == NULL) {
        printf("[E] ELINK_PING$ ping handle malloc failed\n");
        return NULL;
    }
    // 将对象加入链表
    if (g_ping_list == NULL) {
        g_ping_list = list_new();
        if (g_ping_list == NULL) {
            vEventGroupDelete(handle->event);
            free(handle);
            printf("[E] ELINK_PING$ ping list malloc failed\n");
            return NULL;
        }
    }
    list_node_t *node = list_node_new((void*)handle);
    if (node == NULL) {
        vEventGroupDelete(handle->event);
        free(handle);
        printf("[E] ELINK_PING$ ping node malloc failed\n");
        return NULL;
    }
    list_node_t *retval = list_rpush(g_ping_list, node);
    if (retval != node) {
        vEventGroupDelete(handle->event);
        free(handle);
        printf("[E] ELINK_PING$ list rpush failed\n");
        return NULL;
    }
    // 启动定时器
    xTimerStart(handle->timer, portMAX_DELAY);
    printf("[I] ELINK_PING$ ping started, sys_id: %d, com_id: %d, timeout: %dms\n", sys_id, com_id, timeout_ms);
    // 发送数据
    if (g_tx_proxy_list[channel] != NULL) {
        mavlink_message_t msg;
        uint16_t pkg_len = elink_ping_get_msg(handle, &msg);
        g_tx_proxy_list[channel](channel, &msg,pkg_len);
    }
    else {
        printf("[E] ELINK_PING$ tx proxy not set\n");
    }
    // 返回句柄
    return (elink_ping_handle_t)handle;
}

/**
 * @brief 挂起程序，等待ping结果到来
 * 
 * @param handle 
 * @return BaseType_t pdTRUE成功, pdFALSE失败
 */
BaseType_t elink_ping_promise(elink_ping_handle_t handle)
{
    // 等待ping事件到来
    elink_ping_handle_internal_t *handle_internal = (elink_ping_handle_internal_t *)handle;
    if (handle_internal == NULL || handle_internal->event == NULL) {
        printf("[E] ELINK_PING$ invalid handle\n");
        return pdFALSE;
    }
    EventBits_t event = xEventGroupWaitBits(
        handle_internal->event,
        PING_FINISH_EVENT_BIT | PING_TIMEOUT_EVENT_BIT,
        pdTRUE,
        pdFALSE,
        portMAX_DELAY
    );
    // 释放资源
    elink_handle_destroy(handle_internal);
    // 判断事件
    if (event & PING_FINISH_EVENT_BIT) {
        printf("[I] ELINK_PING$ ping finished\n");
        return pdTRUE;
    }
    printf("[W] ELINK_PING$ ping failed\n");
    return pdFALSE;
}

/**
 * @brief MAVLink消息接收代理，检查ping结果
 * 
 * @param channel 信道编号，每个信道只能有一个rx_proxy
 * @param msg 该信道上接收到的消息
 * 
 * @note 信道是物理概念而不是抽象概念。比如一个UART就是一个信道，不应承担两个信道的职责。
 * @note 请将本函数放在指定信道的接收回调函数中，并且信道ID不能冲突，否则会导致ping数据从不正确的通道发送
 */
void elink_ping_rx_proxy(mavlink_channel_t channel,const mavlink_message_t *msg)
{
    printf("[I] ELINK_PING$ rx proxy for %hhd called\n",channel);

    mavlink_ping_t ping_msg;
    mavlink_msg_ping_decode(msg, &ping_msg);
    printf("[D] ELINK_PING$ decode finish,msg time = %" PRIu64 "\n",ping_msg.time_usec);

    // 情况1：处理别人ping自己
    if (msg->msgid == MAVLINK_MSG_ID_PING && 
        ping_msg.target_system == ELINK_MAVLINK_SYS_ID && 
        ping_msg.target_component == ELINK_MAVLINK_COMP_ID
    ) {
        printf("[D] ELINK_PING$ others is pinging local\n");
        mavlink_message_t pong_msg;
        uint16_t pkg_len = mavlink_msg_ping_pack(
            ELINK_MAVLINK_SYS_ID,     // 本地系统
            ELINK_MAVLINK_COMP_ID,    // 本地组件
            &pong_msg,                // 需要发送的报文
            ELINK_PING_GET_TIMSTAMP(),// 本地时间戳
            ping_msg.seq,             // ping发起方的序列号
            ping_msg.target_system,   // ping发起方ping的系统
            ping_msg.target_component // ping发起方ping的组件
        );
        if (g_tx_proxy_list[channel] != NULL) {
            g_tx_proxy_list[channel](channel, &pong_msg, pkg_len);
            printf("[I] ELINK_PING$ pong is broadcasted\n");
        }
        return;
    }
    // 情况2：处理别人的pong
    if (msg->msgid == MAVLINK_MSG_ID_PING) {
        printf("[D] ELINK_PING$ others is ponging local\n");
        list_node_t *itr = g_ping_list->head;
        while (itr != NULL) {
            printf("[D] ELINK_PING$ itr start.\n");
            elink_ping_handle_internal_t *handle = (elink_ping_handle_internal_t *)itr->val;
            if (handle->sys_id == ping_msg.target_system && 
                handle->com_id == ping_msg.target_component
            ) {
                list_node_t *itr_tobe_delete = itr;
                itr = itr->next; // 先迭代，避免因为释放资源造成野指针！！！！
                // 匹配上了，证明是别人pong了自己，先置位，然后释放资源
                xEventGroupSetBits(handle->event, PING_FINISH_EVENT_BIT);
                xTimerStop(handle->timer, portMAX_DELAY);
                list_remove(g_ping_list, itr_tobe_delete);
                printf("[I] ELINK_PING$ pong recieved sys_id: %hhd, com_id: %hhd\n", handle->sys_id, handle->com_id);
            }
            else {
                printf("[D] ELINK_PING$ itr not match, sys_id: %hhd, com_id: %hhd\n", handle->sys_id, handle->com_id);
                itr = itr->next;
            }
        }
        return;// 不在while中return，处理重复情况
    }
}

/**
 * @brief ping消息发送代理，响应外界的ping请求
 * 
 * @param channel 信道编号，每个信道只能有一个tx_proxy
 * @param cb 该信道的发送函数
 * @note 请自行实现每个信道的发送函数，并将需要支持PING的信道的发送函数用本函数注册
 */
void elink_ping_tx_proxy_set(mavlink_channel_t channel,elink_ping_tx_callback_t cb)
{
    if (cb == NULL) {
        printf("[E] ELINK_PING$ tx proxy callback is NULL\n");
    }
    g_tx_proxy_list[channel] = cb;
}

/* 私有函数定义 ---------------------------------------------------------------------*/

/**
 * @brief 创建一个ping对象
 * 
 * @param handle 
 */
static void elink_handle_create(elink_ping_handle_internal_t **handle,uint8_t sys_id,MAV_COMPONENT com_id,uint16_t timeout_ms)
{
    // 申请一个新的ping对象
    *handle = (elink_ping_handle_internal_t *)malloc(sizeof(elink_ping_handle_internal_t));
    if ((*handle) == NULL) {
        printf("[E] ELINK_PING$ event group malloc failed");
        return;
    }
    // 初始化ping对象
    (*handle)->sys_id = sys_id;
    (*handle)->com_id = com_id;
    (*handle)->seq = g_ping_seq;
    g_ping_seq++;
    // 创建事件组
    (*handle)->event = xEventGroupCreate();
    if ((*handle)->event == NULL) {
        free((*handle));
        printf("[E] ELINK_PING$ malloc failed");
        return;
    }
    // 创建定时器
    (*handle)->timer = xTimerCreate(
        "elink_ping_timer",
        pdMS_TO_TICKS(timeout_ms),
        pdFALSE, 
        (void *)(*handle), 
        elink_timer_callback
    );
    if ((*handle)->timer == NULL) {
        vEventGroupDelete((*handle)->event);
        free((*handle));
        printf("[E] ELINK_PING$ timer malloc failed");
        return;
    }
    printf("[I] ELINK_PING$ handle created\n");
}

/**
 * @brief 销毁一个ping对象
 * 
 * @param handle 
 */
static void elink_handle_destroy(elink_ping_handle_internal_t *handle)
{
    if (handle == NULL) return;
    if (handle->event == NULL) return;
    if (handle->timer == NULL) return;

    vEventGroupDelete(handle->event);
    xTimerDelete(handle->timer,portMAX_DELAY);
    free(handle);
}
/**
 * @brief 定时器回调函数，触发PING超时事件
 * 
 * @param xTimer 
 */
static void elink_timer_callback( TimerHandle_t xTimer )
{
    elink_ping_handle_internal_t *handle = (elink_ping_handle_internal_t *)pvTimerGetTimerID(xTimer);
    xEventGroupSetBits(handle->event, PING_TIMEOUT_EVENT_BIT);
}

/**
 * @brief 从初始化的ping服务中获取MAVLink消息
 * 
 * @param handle ping服务
 * @param msg 需要发送的消息
 * @return uint16_t 消息长度
 */
static uint16_t elink_ping_get_msg(elink_ping_handle_t handle,mavlink_message_t *msg)
{
    elink_ping_handle_internal_t *handle_internal = (elink_ping_handle_internal_t *)handle;
    uint64_t timestamp_local = ELINK_PING_GET_TIMSTAMP();
    uint16_t pkg_len = mavlink_msg_ping_pack(
        ELINK_MAVLINK_SYS_ID,
        ELINK_MAVLINK_COMP_ID,
        msg,
        timestamp_local,
        handle_internal->seq,
        handle_internal->sys_id,
        handle_internal->com_id
    );
    return pkg_len;
}