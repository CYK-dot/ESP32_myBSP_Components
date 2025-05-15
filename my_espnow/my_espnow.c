/**
 * @file my_espnow.c
 * @author CYK-Dot
 * @brief 在ESP-NOW之上实现基础的CRC校验和接收回调，
 *        但由于802.11本来就自带CRC，所以本文件用处不大，仅用于代码参考
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
#include "nvs_flash.h"
#include "esp_random.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_now.h"
#include "esp_crc.h"

#include "my_wifi.h"
#include "my_espnow.h"


static const char *TAG = "MyESP-NOW";
static const uint8_t g_macBroadcast[ESP_NOW_ETH_ALEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
//static const uint8_t g_macBroadcastT[ESP_NOW_ETH_ALEN] = {0x12, 0x12, 0x12, 0x12, 0x12, 0x12};
static bool g_txIsMsgPending = false; // 是否有数据尚未发送完成(ESP-NOW文档指出，上次发送未完成就继续发送，会导致回调混乱)

static bool g_isHostAP;
static MyNowMessage_t g_rxBuffer;
static MyNowRxCallback_t g_rxCallback[MY_ESPNOW_CALLBACK_LEN];
static uint8_t g_rxCallbackCount = 0;

static esp_err_t prvParseData(MyNowMessage_t* msg,const uint8_t *inData,size_t inLen);
static esp_err_t prvPackData(const MyNowMessage_t *msg,uint8_t *outData,size_t outLen);
static void MyNowSend_Callback(const uint8_t *mac_addr, esp_now_send_status_t status);
static void MyNowRecv_Callback(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len);

/**
 * @brief 初始化ESP-NOW协议栈
 * 
 * @param isHostAP 是否作为AP热点
 * @param phyMode  wifi收发器模式
 * @param bufferSize 数据包的最大长度，不应小于8，因为数据包中一定有时间戳和CRC
 * @return esp_err_t 
 * 
 * @note 暂时不支持wifi6
 */
esp_err_t MyNowSetup(bool isHostAP,wifi_phy_mode_t phyMode,size_t bufferSize)
{
    ESP_LOGW(TAG,"初始化ESP-NOW协议栈");
    ESP_ERROR_CHECK( esp_now_init() );
    ESP_ERROR_CHECK( esp_now_register_send_cb(MyNowSend_Callback) );
    ESP_ERROR_CHECK( esp_now_register_recv_cb(MyNowRecv_Callback) );
    ESP_ERROR_CHECK( esp_now_set_pmk((uint8_t *)MY_ESPNOW_PMK) );

    ESP_LOGI(TAG,"设置ESP-NOW所需的收发器");
    if (phyMode == WIFI_PHY_MODE_LR) {
        //802.11LR
        // esp_now_rate_config_t now_wifi_config = {
        //     .phymode = phyMode,
        //     .rate = WIFI_PHY_RATE_LORA_500K,
        // };
        // ESP_ERROR_CHECK(esp_now_set_peer_rate_config(g_macBroadcast,&now_wifi_config)); // 未知原因，无法使用
        ESP_ERROR_CHECK(esp_wifi_config_espnow_rate(WIFI_IF_STA,WIFI_PHY_RATE_LORA_250K));
    }
    else if (phyMode == WIFI_PHY_MODE_HT40) {
        //高并发
        // esp_now_rate_config_t now_wifi_config = {
        //     .phymode = phyMode,
        //     .rate = WIFI_PHY_RATE_54M,
        //     .dcm = true,
        //     .ersu = true,
        // };
        // ESP_ERROR_CHECK(esp_now_set_peer_rate_config(g_macBroadcast,&now_wifi_config));
        ESP_ERROR_CHECK(esp_wifi_config_espnow_rate(WIFI_IF_STA,WIFI_PHY_RATE_54M));
    }
    else if (phyMode == WIFI_PHY_MODE_HT20) {
        //高兼容
        // esp_now_rate_config_t now_wifi_config = {
        //     .phymode = phyMode,
        //     .rate = WIFI_PHY_RATE_6M,
        //     .dcm = true,
        //     .ersu = true,
        // };
        // ESP_ERROR_CHECK(esp_now_set_peer_rate_config(g_macBroadcast,&now_wifi_config));
        ESP_ERROR_CHECK(esp_wifi_config_espnow_rate(WIFI_IF_STA,WIFI_PHY_RATE_6M));
    }

    ESP_LOGI(TAG,"初始化ESP-NOW配对");
    esp_now_peer_info_t *peer = (esp_now_peer_info_t*)malloc(sizeof(esp_now_peer_info_t));
    if (peer == NULL) {
        ESP_LOGE(TAG, "生成ESP-NOW配对失败");
        esp_now_deinit();
        return ESP_FAIL;
    }
    memset(peer, 0, sizeof(esp_now_peer_info_t));
    if (isHostAP == true){
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
    g_rxBuffer.data = (uint8_t*)malloc(bufferSize);
    if (g_rxBuffer.data == NULL) {
        ESP_LOGE(TAG,"空间不足，接收缓冲区申请失败");
        return ESP_ERR_NO_MEM;
    }
    g_rxBuffer.maxLen = bufferSize;
    g_rxBuffer.msgLen = bufferSize;

    return ESP_OK;
}

/**
 * @brief 动态创建一条消息
 * 
 * @param maxLen 
 * @return MyNowMessage_t* 
 */
MyNowMessage_t* MyNowMessageCreate(size_t maxLen)
{
    MyNowMessage_t *handle = malloc(sizeof(MyNowMessage_t));
    if (handle == NULL) {
        return NULL;
    }

    handle->data = malloc(maxLen);
    if (handle->data == NULL) {
        free(handle);
        return NULL;
    }

    handle->maxLen = maxLen;
    handle->msgLen = 0;
    handle->tick = 0;
    memset(handle->data,0,maxLen);

    return handle;
}

/**
 * @brief 动态删除一条消息
 * 
 * @param maxLen 
 * @return MyNowMessage_t* 
 */
void MyNowMessageDelete(MyNowMessage_t* msg)
{
    free(msg->data);
    free(msg);
}

/**
 * @brief 通过ESP-NOW发送数据
 * 
 * @param data 
 * @param len 
 * @return esp_err_t 
 */
esp_err_t MyNowSend(const void* data,size_t len)
{
    MyNowMessage_t msg;
    esp_err_t err;

    // 上次发送尚未完成
    if (g_txIsMsgPending == true) {
        return ESP_ERR_NOT_ALLOWED;
    }
    g_txIsMsgPending = true;
    
    //准备数据包
    msg.data = (uint8_t*)data;
    msg.maxLen = len;
    msg.msgLen = len;

    //申请二进制空间
    uint8_t *txData = (uint8_t*)malloc(len + sizeof(suseconds_t) + 2);
    if (txData == NULL) {
        ESP_LOGE(TAG,"堆空间不足，发送失败");
        return ESP_ERR_NO_MEM;
    }

    //填充二进制空间
    err = prvPackData(&msg,txData,len + sizeof(suseconds_t) + 2);
    if (err != ESP_OK) {
        ESP_LOGE(TAG,"打包数据失败");
        return ESP_ERR_NOT_ALLOWED;
    }

    //发送
    err = esp_now_send(g_macBroadcast,(const uint8_t*)txData,len + sizeof(suseconds_t) + 2);
    if (err != ESP_OK) {
        ESP_LOGE(TAG,"发送数据失败，错误码%#X",err);
        return err;
    }

    return ESP_OK;
}

/**
 * @brief 获取当前时刻最新的ESP-NOW数据
 * 
 * @param msg 
 * @return esp_err_t 
 */
esp_err_t MyNowRecv(MyNowMessage_t* msg)
{
    portMUX_TYPE mutex;
    portENTER_CRITICAL(&mutex);
    memcpy(msg,&g_rxBuffer,sizeof(MyNowMessage_t));
    portEXIT_CRITICAL(&mutex);
    return ESP_OK;
}

/**
 * @brief 注册回调函数，以跟踪所有ESP-NOW数据
 * 
 * @param cb 
 * @return esp_err_t 
 */
esp_err_t MyNowRegisterRecv(MyNowRxCallback_t cb)
{
    if (g_rxCallbackCount == MY_ESPNOW_CALLBACK_LEN) {
        ESP_LOGE(TAG,"注册回调函数失败，队列已满");
        return ESP_ERR_NO_MEM;
    }
    if (cb == NULL) {
        ESP_LOGE(TAG,"参数不能为NULL");
        return ESP_ERR_INVALID_ARG;
    }

    g_rxCallback[g_rxCallbackCount] = cb;
    g_rxCallbackCount++;
    ESP_LOGW(TAG,"回调函数注册成功");
    return ESP_OK;
}

/**
 * @brief 输入二进制序列，输出格式化的信息
 * 
 * @param msg 请确保msg来自于MyNowMessageCreateRx函数
 * @param inData 
 * @param inLen 
 * @return esp_err_t 
 */
static esp_err_t prvParseData(MyNowMessage_t* msg,const uint8_t *inData,size_t inLen)
{
    //检查msg能否存放数据
    int dataLen = inLen-2-sizeof(suseconds_t);
    int msgLen =  inLen-2;
    // ESP_LOGW(TAG,"准备解包%d个数据，载荷末尾%d,时间戳末尾%d,载荷%s",
    //     inLen,
    //     dataLen,
    //     msgLen,
    //     (char*)inData);
    if (msg->maxLen < dataLen) {
        //ESP_LOGE(TAG,"待写入消息中允许的最大长度为%d,最小预期为%d",(int)msg->maxLen,dataLen);
        return ESP_ERR_NO_MEM;
    }

    //检查CRC是否正确
    uint16_t crcRecv;
    uint16_t crc;
    memcpy(&crcRecv,&inData[msgLen],sizeof(uint16_t));
    crc = esp_crc16_le(UINT16_MAX, inData, msgLen);
    
    if (crc != crcRecv) {
        ESP_LOGE(TAG,"crc校验失败，实际值%hx,收到值%hx",crc,crcRecv);
        return ESP_ERR_INVALID_CRC;
    }

    //拷贝并赋值数据
    memcpy(msg->data,inData,dataLen);
    msg->msgLen = dataLen;
    memcpy(&msg->tick,&inData[dataLen],sizeof(msg->tick));

    return ESP_OK;
}

/**
 * @brief 输入格式化的信息，输出二进制序列
 * 
 * @param msg 
 * @param outData 
 * @param outLen 
 * @return esp_err_t 
 */
static esp_err_t prvPackData(const MyNowMessage_t *msg,uint8_t *outData,size_t outLen)
{
    suseconds_t tick;
    //检查输出二进制数组长度是否严格等于预期
    if (outLen != msg->msgLen + sizeof(suseconds_t) + 2) {
        ESP_LOGE(TAG,"待打包的数据长度为%d,最大预期为%d",msg->msgLen,outLen-sizeof(suseconds_t)-2);
        return ESP_ERR_NO_MEM;
    }

    //拷贝载荷
    memcpy(outData,msg->data,msg->msgLen);
    //拷贝时间戳
    tick = (suseconds_t)esp_log_timestamp();
    memcpy(&outData[msg->msgLen],&tick,sizeof(tick));
    //拷贝CRC
    uint16_t crc = esp_crc16_le(UINT16_MAX, (const uint8_t*)outData, msg->msgLen+sizeof(suseconds_t));
    memcpy(&outData[msg->msgLen+sizeof(suseconds_t)],&crc,sizeof(uint16_t));
    
    //ESP_LOGI(TAG,"打包完成,时间戳%ld,校验%hx",tick,crc);
    return ESP_OK;
}

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
    }
    else {
        g_txIsMsgPending = false;
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
    static portMUX_TYPE mutex = portMUX_INITIALIZER_UNLOCKED;
    esp_err_t err;
    //ESP_LOGI(TAG,"收到来自%d:%d:%d:%d:%d:%d的数据,长度%d,rssi=%d",recv_info->src_addr[0],recv_info->src_addr[1],recv_info->src_addr[2],recv_info->src_addr[3],recv_info->src_addr[4],recv_info->src_addr[5],len,recv_info->rx_ctrl->rssi);

    portENTER_CRITICAL_ISR(&mutex);

    //ESP_LOGI(TAG,"准备解析数据");
    //解包
    err = prvParseData(&g_rxBuffer,data,len);
    if (err != ESP_OK) {
        portEXIT_CRITICAL_ISR(&mutex);
        return;
    }
    //ESP_LOGI(TAG,"数据解析成功，时间戳%ld,数据长度%d",g_rxBuffer.tick,g_rxBuffer.msgLen);

    //回调
    for(int i=0;i<g_rxCallbackCount;i++) {
        g_rxCallback[i](recv_info->rx_ctrl->rssi,(const MyNowMessage_t*)&g_rxBuffer);
    }
    portEXIT_CRITICAL_ISR(&mutex);
}