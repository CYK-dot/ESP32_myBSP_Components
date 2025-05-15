#pragma once

#include <esp_adc/adc_continuous.h>
#include <freertos/FreeRTOS.h>

#define MY_ADC_CONV_CALLBACK_FUNCTOR(OUTPUT_ARR)             \
do{                                                          \
    BaseType_t mustYield = pdFALSE;                          \
    portMUX_TYPE mutex = portMUX_INITIALIZER_UNLOCKED;       \
    portENTER_CRITICAL_ISR(&mutex);                          \
    memcpy(OUTPUT_ARR,edata->conv_frame_buffer,edata->size); \
    portEXIT_CRITICAL_ISR(&mutex);                           \
    return (mustYield == pdTRUE);                            \
}while(0) 

typedef adc_continuous_handle_t* MyAdc_t;

typedef struct{
    adc_channel_t *channelArr;        ///< 外设配置：数组     使用哪一个通道
    adc_unit_t     periArr;           ///< 外设配置：数组     由于ADC2被wifi占有，因此不允许交叉扫描不同ADC外设
    uint8_t        taskCnt;           ///< 外设配置：数组大小 一次转换包括多少个通道

    uint32_t       convCnt;           ///< 策略配置：多少次转换生成一个ADC帧(一旦一个ADC帧生成，就会调用回调函数)
    adc_digi_convert_mode_t convMode; ///< 策略配置：转换的策略
    adc_continuous_evt_cbs_t cb;      ///< 输出配置：回调函数(暂未使用)
}MyAdcConfig_t;

typedef struct{
    adc_channel_t *channelArr; ///< 哪一个通道
    float         *voltArr;    ///< ADC获取到的值，范围0~1
    uint8_t        taskCnt;    ///< 数组索引最大值
    uint8_t       *prv_buffer; ///<不应主动访问，只用于临时存放数据
}MyAdcDataStatic_Handle;
typedef MyAdcDataStatic_Handle* MyAdcData_t;

// ADC句柄相关
MyAdc_t     MyAdcHandleCreateLoopOnce(const MyAdcConfig_t *conf);
void        MyAdcHandleDelete(MyAdc_t handle);
esp_err_t   MyAdcHandleStart(MyAdc_t handle);
esp_err_t   MyAdcHandleStop(MyAdc_t handle);

// ADC数据相关
MyAdcData_t  MyAdcDataCreate(const MyAdcConfig_t *conf);
void         myAdcDataDelete(MyAdcData_t handle);

// 数据获取
//esp_err_t MyAdcDataGetMean(myAdc_t handle,MyAdcData_t data);