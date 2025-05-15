/**
 * @file my_espnow_remote.h
 * @author CYK-Dot
 * @brief 在ESP-NOW之上实现带端口-订阅-发布机制的无连接协议，称为Now-Remote协议
 * @version 0.1
 * @date 2025-05-15
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once

#include <stdint.h>
#include <time.h>
#include <esp_err.h>

#define MY_ESPNOW_REMOTE_PMK   "LMK_FILGHT12345" ///< esp-now配对码
#define MY_ESPNOW_REMOTE_ERR_TX 1 ///< 错误回调码

typedef struct 
{
    uint8_t srcPort;
    uint8_t dstPort;
}NowRemoteAddr_t;

typedef struct 
{
    void *payload;
    uint16_t size;
}NowRemoteMessage_t;

typedef struct 
{
    suseconds_t tick;
    uint32_t rssi;
}NowRemoteCtrl_t;

typedef struct 
{
   bool isHostAP;
   wifi_phy_mode_t phyMode;
   size_t localPortMaxCount;
   size_t payloadMaxSize;
}NowRemoteConf_t;

typedef void (*NowRemoteSubscriber_fptr) (NowRemoteMessage_t,NowRemoteAddr_t addr,NowRemoteCtrl_t ctrl);
typedef void (*NowRemoteErrorCallback_fptr)(uint8_t,uint32_t);

// 驱动程序
esp_err_t NowRemoteProto_Init(const NowRemoteConf_t *conf);
esp_err_t NowRemoteProto_RegisterTxFailCallback(NowRemoteErrorCallback_fptr cb);
esp_err_t NowRemoteProto_Subscribe(uint8_t localPort,NowRemoteSubscriber_fptr cb);
esp_err_t NowRemoteProto_Public(NowRemoteAddr_t addr,NowRemoteMessage_t msg,size_t tickToWait);

// 样例程序
void NowRemoteProto_ExampleHostInit(void);
void NowRemoteProto_ExampleHostMain(const char *strToSend);
void NowRemoteProto_ExampleSlaveInit(NowRemoteSubscriber_fptr yourCallback);
void NowRemoteProto_ExampleSlaveMain(void);
