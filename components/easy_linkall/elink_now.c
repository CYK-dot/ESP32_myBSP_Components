/**
 * @file elink_now.c
 * @author CYK-Dot
 * @brief 基于MavLink的通用多信道通信--ESPNOW实现
 * @version 0.1
 * @date 2025-06-26
 *
 * @copyright Copyright (c) 2025
 */

/* 头文件引入 -----------------------------------------------------------------------*/

#include <esp_err.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <esp_wifi.h>
#include <esp_now.h>

#include <esp_comm/mavlink.h>
#include <mavlink_helpers.h> //仅为了避免编译报错，本文件并无作用

#include "elink_global.h"
#include "elink_ping.h"
#include "elink_now.h"

/* 私有类型定义 ---------------------------------------------------------------------*/

/* 私有宏定义 -----------------------------------------------------------------------*/

static const char* TAG = "elink_now";

/* 全局变量声明 ---------------------------------------------------------------------*/

// ESP-NOW相关
static bool g_isHostAP = false;
static uint8_t g_mac_broadcast[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; 
static uint8_t g_tx_buffer[MAVLINK_MAX_PACKET_LEN];

// MavLink相关
static enowlink_conf_t g_conf;
static mavlink_status_t g_rx_status; // 全局变量，确保状态机的状态不丢失

/* 私有函数声明 ---------------------------------------------------------------------*/

static void elink_now_send_callback(const uint8_t *mac_addr, esp_now_send_status_t status);
static void elink_now_recv_callback(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len);
static BaseType_t elink_now_tx_proxy(mavlink_channel_t channel,mavlink_message_t *msg,uint16_t pkg_len);

/* 导出函数定义 ---------------------------------------------------------------------*/

/**
 * @brief 将ESP-NOW初始化为一个MAVLink信道
 * 
 * @param conf 
 * @return esp_err_t 
 */
esp_err_t elink_now_init(const enowlink_conf_t* conf)
{
    ESP_LOGW(TAG,"初始化ESP-NOW协议栈,请确保在此前wifi完成配置并且尚未启动");
    ESP_ERROR_CHECK( esp_now_init() );
    ESP_ERROR_CHECK( esp_now_register_send_cb(elink_now_send_callback) );
    ESP_ERROR_CHECK( esp_now_register_recv_cb(elink_now_recv_callback) );
    ESP_ERROR_CHECK( esp_now_set_pmk((uint8_t *)ELINK_NOW_REMOTE_PMK) );

    ESP_LOGI(TAG,"初始化ESP-NOW配对");
    esp_now_peer_info_t *peer = (esp_now_peer_info_t*)malloc(sizeof(esp_now_peer_info_t));
    if (peer == NULL) {
        ESP_LOGE(TAG, "生成ESP-NOW配对失败");
        esp_now_deinit();
        return ESP_FAIL;
    }
    memset(peer, 0, sizeof(esp_now_peer_info_t));
    if (conf->isHostAP == true){
        peer->ifidx = WIFI_IF_AP;
        g_isHostAP = true;
    }
    else {
        peer->ifidx = WIFI_IF_STA;
        g_isHostAP = false;
    }
    peer->encrypt = false;
    memcpy(peer->peer_addr, g_mac_broadcast, ESP_NOW_ETH_ALEN);
    ESP_ERROR_CHECK( esp_now_add_peer(peer) );
    free(peer);

    ESP_LOGI(TAG,"初始化MAVLink协议栈");
    memcpy(&g_conf, conf, sizeof(enowlink_conf_t));
    elink_ping_tx_proxy_set(conf->ch_id, elink_now_tx_proxy);

    ESP_LOGW(TAG,"初始化MAVLINK-NOW成功");
    return ESP_OK;
}

/**
 * @brief 通过ESP-NOW信道ping其他设备(发起)
 * 
 * @param system_id 
 * @param component_id 
 * @param timeout_ms 
 * @return elink_ping_handle_t 
 */
elink_ping_handle_t elink_now_ping_start(uint8_t system_id,uint8_t component_id,uint16_t timeout_ms)
{
    return elink_ping_start(g_conf.ch_id,system_id,component_id,timeout_ms);
}
/**
 * @brief 通过ESP-NOW信道ping其他设备(等待并检查)
 * 
 * @param handle 
 * @return esp_err_t 
 */
esp_err_t elink_now_ping_promise(elink_ping_handle_t handle)
{
    esp_err_t err = elink_ping_promise(handle)==true? ESP_OK:ESP_FAIL;
    return err;
}


/* 私有函数定义 ---------------------------------------------------------------------*/
/**
 * @brief 发送完成回调函数
 * 
 * @param mac_addr 
 * @param status 
 */
static void elink_now_send_callback(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    
}

/**
 * @brief 接收完成回调函数
 * 
 * @param recv_info 
 * @param data 
 * @param len 
 */
static void elink_now_recv_callback(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len)
{
    mavlink_message_t msg;
    bool mavlink_status;
    // 将ESP-NOW数据包解析为MAVLink数据包
    for(uint16_t i=0;i<len;i++) {
        mavlink_status = mavlink_parse_char(
            g_conf.ch_id,
            data[i],
            &msg,
            &g_rx_status
        );
        // 成功解析
        if (mavlink_status == true) {
            ESP_LOGI(TAG, "MAVLink解析成功");
            if (g_conf.enablePing) 
                elink_ping_rx_proxy(g_conf.ch_id, &msg);
        }
        // 解析失败
        else {
            memset(&g_rx_status, 0, sizeof(g_rx_status));
            ESP_LOGW(TAG, "MAVLink解析失败,或许只是原始的ESP-NOW帧,或者出现了传输差错");
        }
    }
}

/**
 * @brief 通过ESP-NOW信道代理MAVLink数据包
 * 
 * @param channel 
 * @param msg 
 * @param pkg_len 
 * @return BaseType_t 
 */
static BaseType_t elink_now_tx_proxy(mavlink_channel_t channel,mavlink_message_t *msg,uint16_t pkg_len)
{
    if (g_conf.ch_id != channel) {
        return pdFALSE;
    }
    uint16_t len = mavlink_msg_to_send_buffer(g_tx_buffer, msg);
    esp_err_t err = esp_now_send(g_mac_broadcast, g_tx_buffer, len);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "MAVLink发送失败");
        return pdFALSE;
    }
    return pdTRUE;
}

/* 范例函数定义 -----------------------------------------------------------------*/

#include "ewifi_basic.h"

/**
 * @brief MAVLink运行于ESP-NOW的样例代码(主机)
 * 
 * @return esp_err_t 
 * @warning 请在elink_global.h中，将从机的[sys,com]配置成[1,2]，并将主机的[sys,com]配置成[1,1]
 */
esp_err_t elink_now_init_example_host(void)
{
    // 配置wifi的物理层协议
    ewifi_conf_t wifi_conf;
    wifi_tx_rate_config_t tx_rate_conf = {
        .dcm = true,  //只有802.11ax可以dcm
        .ersu = true, //只有802.11ax可以ersu
        .phymode = WIFI_PHY_MODE_LR,
        .rate = WIFI_PHY_RATE_LORA_250K
    };
    ewifi_basic_get_conf_from_default_ap(&wifi_conf);
    ewifi_basic_set_ap_peri_ssid_password(&wifi_conf,"ESP32_Now_Test","12345678");
    ewifi_basic_set_ap_ptl_lr(&wifi_conf);
    ewifi_basic_set_ap_phy_fix_rate(&wifi_conf,&tx_rate_conf);
    ewifi_basic_print_conf(&wifi_conf);
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    ewifi_basic_init(&wifi_conf);
    // 配置链路层协议
    enowlink_conf_t conf = {
        .ch_id = 0,
        .isHostAP = true,
        .enablePing = true
    };
    elink_now_init(&conf);
    // 联网
    ewifi_wait_connection(WIFI_MODE_AP);
    vTaskDelay(pdMS_TO_TICKS(500));
    ESP_LOGW(TAG,"样例代码初始化完成！");
    // ping对方
    ESP_LOGI(TAG,"开始ping从机");
    elink_ping_handle_t handle = elink_ping_start(conf.ch_id,1,2,UINT16_MAX);
    BaseType_t err = elink_ping_promise(handle);
    ESP_LOGW(TAG,"ping结果:%s",err==pdTRUE?"成功":"失败");

    while(1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

/**
 * @brief MAVLink运行于ESP-NOW的样例代码(从机)
 * 
 * @return esp_err_t 
 * @warning 请在elink_global.h中，将从机的[sys,com]配置成[1,2]，并将主机的[sys,com]配置成[1,1]
 */
esp_err_t elink_now_init_example_client(void)
{
    // 配置wifi的物理层协议
    ewifi_conf_t wifi_conf;
    wifi_tx_rate_config_t tx_rate_conf = {
        .dcm = true,  //只有802.11ax可以dcm
        .ersu = true, //只有802.11ax可以ersu
        .phymode = WIFI_PHY_MODE_LR,
        .rate = WIFI_PHY_RATE_LORA_250K
    };
    ewifi_basic_get_conf_from_default_sta(&wifi_conf);
    ewifi_basic_set_sta_peri_ssid_password(&wifi_conf,"ESP32_Now_Test","12345678");
    ewifi_basic_set_sta_ptl_lr(&wifi_conf);
    ewifi_basic_set_sta_phy_fix_rate(&wifi_conf,&tx_rate_conf);
    ewifi_basic_print_conf(&wifi_conf);
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    ewifi_basic_init(&wifi_conf);
    // 配置链路层协议
    enowlink_conf_t conf = {
        .ch_id = 0,
        .isHostAP = false,
        .enablePing = true
    };
    elink_now_init(&conf);
    // 联网
    ewifi_wait_connection(WIFI_MODE_STA);
    vTaskDelay(pdMS_TO_TICKS(500));
    ESP_LOGW(TAG,"样例代码初始化完成！开始等待被主机ping");
    while(1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}