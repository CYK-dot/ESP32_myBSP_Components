/**
 * @file my_espnow_remote.c
 * @author CYK-Dot
 * @brief 在ESP-NOW之上实现带端口-订阅-发布机制的无连接协议，称为Now-Remote协议
 * @version 0.1
 * @date 2025-05-15
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <assert.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "freertos/task.h"
#include <freertos/event_groups.h>
#include "nvs_flash.h"
#include "esp_random.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_now.h"
#include "esp_crc.h"

#include "ewifi_basic.h"
#include "eespnow.h"

/* 私有宏定义 ---------------------------------------------------------------------------------------*/
#define NOW_REMOTE_EVENT_TX_FINISH (1 << 0)
#define NOW_REMOTE_EVENT_TX_FAIL   (1 << 1)

/* 静态函数声明 -------------------------------------------------------------------------------------*/

static void MyNowSend_Callback(const uint8_t *mac_addr, esp_now_send_status_t status);
static void MyNowRecv_Callback(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len);

/* 静态变量声明 -------------------------------------------------------------------------------------*/

static const char *TAG = "NowRemote";
static bool g_isHostAP;
static EventGroupHandle_t g_nowRemoteEvent;
static NowRemoteErrorCallback_fptr g_errCallback = NULL;

static const uint8_t g_macBroadcast[ESP_NOW_ETH_ALEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
// ESP-NOW文档指出，不建议在上一帧发送完成之前发送下一帧，因此类似HAL库的接收缓冲区是完全可行的，由本协议负责互斥管理
static uint8_t *g_txBuffer; 
static uint16_t g_txMaxPayloadSize;

static NowRemoteSubscriber_fptr *g_rxCallbackSubscribed;
static uint8_t *g_rxPortSubscribed;
static size_t g_rxPortMaxCount;
static size_t g_rxPortNowCount;

/* 导出函数定义 -------------------------------------------------------------------------------------*/

/**
 * @brief 初始化Now-Remote协议栈
 * 
 * @param conf 
 * @return esp_err_t 
 * 
 * @note 使用前，请先确保使用my_wifi完成物理层的配置，本函数要晚于wifi_init被调用
 * @todo 当初始化失败时会内存泄露，不想管
 */
esp_err_t NowRemoteProto_Init(const NowRemoteConf_t *conf)
{
    ESP_LOGW(TAG,"初始化ESP-NOW协议栈");
    ESP_ERROR_CHECK( esp_now_init() );
    ESP_ERROR_CHECK( esp_now_register_send_cb(MyNowSend_Callback) );
    ESP_ERROR_CHECK( esp_now_register_recv_cb(MyNowRecv_Callback) );
    ESP_ERROR_CHECK( esp_now_set_pmk((uint8_t *)MY_ESPNOW_REMOTE_PMK) );

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
    memcpy(peer->peer_addr, g_macBroadcast, ESP_NOW_ETH_ALEN);
    ESP_ERROR_CHECK( esp_now_add_peer(peer) );
    free(peer);

    ESP_LOGI(TAG,"初始化接收队列");
    g_rxCallbackSubscribed = (NowRemoteSubscriber_fptr *)malloc(sizeof(NowRemoteSubscriber_fptr) * conf->localPortMaxCount);
    g_rxPortSubscribed = (uint8_t *)malloc(sizeof(uint8_t) * conf->localPortMaxCount);
    g_rxPortMaxCount = conf->localPortMaxCount;
    g_rxPortNowCount = 0;
    if (g_rxCallbackSubscribed == NULL || g_rxPortSubscribed == NULL) {
        ESP_LOGE(TAG,"空间不足，接收缓冲区申请失败");
        return ESP_ERR_NO_MEM;
    }

    ESP_LOGI(TAG,"初始化事件组");
    g_nowRemoteEvent = xEventGroupCreate();
    if (g_nowRemoteEvent == NULL) {
        ESP_LOGE(TAG,"空间不足，接收缓冲区申请失败");
        return ESP_ERR_NO_MEM;
    }
    xEventGroupSetBits(g_nowRemoteEvent,NOW_REMOTE_EVENT_TX_FINISH);

    ESP_LOGI(TAG,"初始化发送缓冲区");
    g_txBuffer = (uint8_t *)malloc(6 + conf->payloadMaxSize);
    g_txMaxPayloadSize = conf->payloadMaxSize;
    if (g_txBuffer == NULL) {
        ESP_LOGE(TAG,"空间不足，接收缓冲区申请失败");
        return ESP_ERR_NO_MEM;
    }
    return ESP_OK;
}

/**
 * @brief 注册发送失败回调函数
 * 
 * @param cb(错误码ID,时间戳) 
 * @return esp_err_t 
 */
esp_err_t NowRemoteProto_RegisterTxFailCallback(NowRemoteErrorCallback_fptr cb)
{
    if (g_errCallback != NULL) {
        ESP_LOGW(TAG,"发送错误回调函数之前注册过了，注册失败");
        return ESP_ERR_NOT_ALLOWED;
    }
    g_errCallback = cb;
    return ESP_OK;
}

/**
 * @brief 向某个端口注册回调函数
 * 
 * @param localPort 
 * @param cb 
 * @return esp_err_t 
 */
esp_err_t NowRemoteProto_Subscribe(uint8_t localPort,NowRemoteSubscriber_fptr cb)
{
    if (g_rxPortNowCount == g_rxPortMaxCount) {
        ESP_LOGE(TAG,"订阅超出限制");
        return ESP_ERR_NO_MEM;
    }
    g_rxCallbackSubscribed[g_rxPortNowCount] = cb;
    g_rxPortSubscribed[g_rxPortNowCount] = localPort;
    g_rxPortNowCount++;
    return ESP_OK;
}

/**
 * @brief 通过某个端口向远端端口发送数据
 * 
 * @param addr 
 * @param msg 
 * @param tickToWait 
 * @return esp_err_t 
 */
esp_err_t NowRemoteProto_Public(NowRemoteAddr_t addr,NowRemoteMessage_t msg,size_t tickToWait)
{
    // 预防数据过长
    if (msg.size > g_txMaxPayloadSize) {
        ESP_LOGE(TAG,"载荷长度过长，无法发送");
        return ESP_ERR_INVALID_ARG;
    }
    // 等待资源可用
    xEventGroupWaitBits(g_nowRemoteEvent,
        NOW_REMOTE_EVENT_TX_FINISH,
        pdTRUE,
        pdFALSE,
        tickToWait);
    // 获取本地时间
    uint32_t tick = esp_log_timestamp();
    // 使用发送缓冲区发送数据
    g_txBuffer[0] = addr.dstPort;
    g_txBuffer[1] = addr.srcPort;
    g_txBuffer[2] = (tick & 0x000000FF);
    g_txBuffer[3] = (tick & 0x0000FF00) >> 8;
    g_txBuffer[4] = (tick & 0x00FF0000) >> 16;
    g_txBuffer[5] = (tick & 0xFF000000) >> 24;
    memcpy(&g_txBuffer[6],msg.payload,msg.size);
    // 发送
    esp_err_t err = esp_now_send(g_macBroadcast,(const uint8_t*)g_txBuffer,msg.size + 6);
    if (err != ESP_OK) {
        ESP_LOGE(TAG,"发送数据失败，错误码%#X",err);
        return err;
    }
    return ESP_OK;
}

/**
 * @brief 例程代码：让ESP32作为Now-Remote发布者，向外发布数据
 * @note 仅需调用该函数即可完成全部初始化，无需额外初始化wifi
 */
void NowRemoteProto_ExampleHostInit(void)
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
    NowRemoteConf_t conf = {
        .isHostAP = true,
        .localPortMaxCount = 1,
        .payloadMaxSize = 25,
        .phyMode = WIFI_PHY_MODE_HT40,
    };
    NowRemoteProto_Init(&conf);
    // 联网
    ewifi_wait_connection(WIFI_MODE_AP);
    ESP_LOGW(TAG,"样例代码初始化完成！");
}

/**
 * @brief 例程代码：发布者在while(1)中的代码
 * @note 请直接放入while(1)中，然后自行vTaskDelay
 * 
 * @param strToSend 
 */
void NowRemoteProto_ExampleHostMain(const char *strToSend)
{
    NowRemoteAddr_t addr = {.dstPort = 0x05,.srcPort = 0x01};
    NowRemoteMessage_t msg = {.payload = (void*)strToSend,.size = strlen(strToSend)+1};
    NowRemoteProto_Public(addr,msg,portMAX_DELAY);
}

/**
 * @brief 例程代码：让ESP32作为Now-Remote订阅者，通过回调函数订阅数据
 * 
 * @param yourCallback 
 */
void NowRemoteProto_ExampleSlaveInit(NowRemoteSubscriber_fptr yourCallback)
{
    // 配置wifi的物理层协议
    ewifi_conf_t wifi_conf;
    wifi_tx_rate_config_t tx_rate_conf = {
        .dcm = false,  //只有802.11ax可以dcm
        .ersu = false, //只有802.11ax可以ersu
        .phymode = WIFI_PHY_MODE_LR,
        .rate = WIFI_PHY_RATE_LORA_250K
    };
    ewifi_basic_get_conf_from_default_sta(&wifi_conf);
    ewifi_basic_set_sta_peri_ssid_password(&wifi_conf,"ESP32_Now_Test","12345678");
    ewifi_basic_set_sta_ptl_lr(&wifi_conf);
    ewifi_basic_set_sta_phy_fix_rate(&wifi_conf,&tx_rate_conf);
    ewifi_basic_init(&wifi_conf);

    // 配置链路层协议
    NowRemoteConf_t conf = {
        .isHostAP = false,
        .localPortMaxCount = 1,
        .payloadMaxSize = 25,
        .phyMode = WIFI_PHY_MODE_HT40,
    };
    NowRemoteProto_Init(&conf);
    NowRemoteProto_Subscribe(0x05,yourCallback);
    // 联网
    ewifi_wait_connection(WIFI_MODE_STA);
    ESP_LOGW(TAG,"样例代码初始化完成！");
}

/**
 * @brief 例程代码：订阅者者在while(1)中的代码
 * @note 请直接放入while(1)中，然后自行vTaskDelay
 * 
 */
void NowRemoteProto_ExampleSlaveMain(void)
{
    ;
}

/* 静态函数定义 -------------------------------------------------------------------------------------*/

/**
 * @brief 发送完成回调函数
 * 
 * @param mac_addr 
 * @param status 
 */
static void MyNowSend_Callback(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    if (status == ESP_NOW_SEND_FAIL){
        ESP_LOGE(TAG,"发送失败");
        xEventGroupClearBits(g_nowRemoteEvent,NOW_REMOTE_EVENT_TX_FINISH);
        xEventGroupSetBits(g_nowRemoteEvent,NOW_REMOTE_EVENT_TX_FAIL);
        if (!g_errCallback)
            g_errCallback(0,esp_log_timestamp());
    }
    else {
        xEventGroupSetBits(g_nowRemoteEvent,NOW_REMOTE_EVENT_TX_FINISH);
        xEventGroupClearBits(g_nowRemoteEvent,NOW_REMOTE_EVENT_TX_FAIL);
    }
}

/**
 * @brief 接收完成回调函数
 * 
 * @param recv_info 
 * @param data 
 * @param len 
 */
static void MyNowRecv_Callback(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len)
{
    //static portMUX_TYPE mutex = portMUX_INITIALIZER_UNLOCKED;
    ESP_LOGD(TAG,"收到来自%d:%d:%d:%d:%d:%d的数据,长度%d,rssi=%d",recv_info->src_addr[0],recv_info->src_addr[1],recv_info->src_addr[2],recv_info->src_addr[3],recv_info->src_addr[4],recv_info->src_addr[5],len,recv_info->rx_ctrl->rssi);
    //portENTER_CRITICAL_ISR(&mutex);

    //解包
    NowRemoteAddr_t addr = {.dstPort=0,.srcPort=0};
    addr.dstPort = data[0];
    addr.srcPort = data[1];

    NowRemoteCtrl_t ctrl = {.rssi=0,.tick=0};
    ctrl.rssi  = recv_info->rx_ctrl->rssi;
    ctrl.tick |= ((uint32_t)data[2]);
    ctrl.tick |= ((uint32_t)data[3]) << 8ULL;
    ctrl.tick |= ((uint32_t)data[4]) << 16ULL;
    ctrl.tick |= ((uint32_t)data[5]) << 24ULL;

    NowRemoteMessage_t msg = {.payload=NULL,.size=0};
    msg.payload = (void*)&data[6];
    msg.size = len - 6;

    //回调
    for(int i=0;i<g_rxPortNowCount;i++) {
        if (addr.dstPort == g_rxPortSubscribed[i])
            g_rxCallbackSubscribed[i](msg,addr,ctrl);
    }

    //portEXIT_CRITICAL_ISR(&mutex);
}