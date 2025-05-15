#pragma once

#include <esp_err.h>

#include "rdlc.h"

typedef struct 
{
    uint16_t rdlcMsgSize;
    uint32_t rdlcBaud;
}CenterConf_t;

// 外设控制接口
esp_err_t BoardCenterCtrl_Init(const CenterConf_t *conf);
esp_err_t BoardCenterCtrl_Ch32Send(RdlcAddr_t addr,const void *data,size_t size,int tickToWait);
esp_err_t BoardCenterCtrl_Ch32RegisterRxCallback(RdlcOnParse_fptr callback);

// 实现的功能1：CH32负责8通道ADC采样和LED闪烁
typedef struct 
{
    uint8_t adcSrcPort;
    uint8_t adcDstPort;
    uint8_t ledSrcPort;
    uint8_t ledDstPort;
}CenterAdceConf_t;

typedef struct 
{
   uint16_t ch0;
   uint16_t ch1;
   uint16_t ch2;
   uint16_t ch3;
   uint16_t ch4;
   uint16_t ch5;
   uint16_t ch6;
   uint16_t ch7;
}CenterRecvData_t;

typedef enum
{
    LED_OFF = 0,
    LED_ON,
    LED_BLINK_ONCE,
    LED_BLINK_TWICE,
    LED_BLINK_THIRD,
    LED_BLINK_FAST,
}CenterLed_t;

#define ADCE_CONF_CEN {.rdlcBaud=230400,.rdlcMsgSize=14}
#define ADCE_CONF_ADCE_DEFAULT {.adcSrcPort=0x01,adcDstPort=0x01,.ledSrcPort=0x01,ledDstPort=0x01}

esp_err_t BoardCenterApp_AdceInit(const CenterConf_t *confCtrl,const CenterAdceConf_t *confApp);
esp_err_t BoardCenterApp_AdceMain(void);

esp_err_t BoardCenterApp_AdceGetAdc(CenterRecvData_t *data);
esp_err_t BoardCenterApp_AdceSetLed(CenterLed_t LED_BLINK_x);