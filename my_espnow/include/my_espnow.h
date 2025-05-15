#pragma once

#ifdef __cplusplus
    extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <time.h>


///< 配置：ESP-NOW协议的配对钥
#define MY_ESPNOW_PMK          "LMK_FILGHT12345"
///< 配置：最大允许注册的回调函数长度
#define MY_ESPNOW_CALLBACK_LEN 3

typedef struct{
    uint8_t  *data;  ///< 数据本体
    suseconds_t tick;///< 时间戳
    size_t   maxLen; ///< 最大报文长度
    size_t   msgLen; ///< 本条报文长度
}MyNowMessage_t;

typedef void (*MyNowRxCallback_t) (int rssi,const MyNowMessage_t*);

MyNowMessage_t* MyNowMessageCreate(size_t maxLen);
void MyNowMessageDelete(MyNowMessage_t* msg);

esp_err_t MyNowSetup(bool isHostAP,wifi_phy_mode_t phyMode,size_t bufferSize);
esp_err_t MyNowSend(const void* data,size_t len);
esp_err_t MyNowRecv(MyNowMessage_t* msg);
esp_err_t MyNowRegisterRecv(MyNowRxCallback_t cb);

#ifdef __cplusplus
    }
#endif