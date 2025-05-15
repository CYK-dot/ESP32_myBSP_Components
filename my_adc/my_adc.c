/**
 * @file my_adc.c
 * @brief 利用DMA批量获取多个通道、多次转换的ADC数据
 * @version 0.1
 * @date 2025-04-23
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include <string.h>
#include <stdio.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <esp_adc/adc_continuous.h>
#include <soc/soc_caps.h>

#include "my_adc.h"


static const char *TAG = "MY-ADC";
                           
/**
 * @brief 创建一个ADC外设。任何时刻只存在一个ADC帧，完成转换后会覆写原先的帧，因此称为LoopOnce。
 * 
 * @note 需手动调用adc_continuous_start启动ADC
 * 
 * @note 只需要最新数据的回调写法：不注册任何回调函数。注意，不应通过回调函数更新数据，因为转换太快了，会消耗CPU。
 * @note 需要全部数据的回调写法：暂时没考虑那么多，不支持。
 */
MyAdc_t MyAdcHandleCreateLoopOnce(const MyAdcConfig_t *conf)
{
    ESP_LOGW(TAG,"开始创建ADC批量转换句柄");
    adc_continuous_handle_t *handle = malloc(sizeof(adc_continuous_handle_t));
    if (handle == NULL) {
        ESP_LOGE(TAG,"内存不足，无法申请ADC句柄");
        return NULL;
    }
    
    // step1: 配置ADC通道的基本参数
    adc_digi_pattern_config_t adc_pattern[SOC_ADC_PATT_LEN_MAX] = {};
    // for (int i = 0; i < conf->channelCnt; i++) {
    //     adc_pattern[i].atten   = ADC_ATTEN_DB_12;             ///< 衰减11倍，原始ADC支持1100mV最大
    //     adc_pattern[i].channel = conf->channelArr[i] & 0x7;   ///< 指定通道
    //     adc_pattern[i].unit    = conf->periArr[i];            ///< 指定ADC外设
    //     adc_pattern[i].bit_width = SOC_ADC_DIGI_MAX_BITWIDTH; ///< 指定输出位数
    // }

    // step2: 配置ADC外设的基本参数
    ESP_LOGI(TAG,"开始创建ADC连续转换所需的配置");
    adc_continuous_handle_cfg_t adc_config = {
        .max_store_buf_size = SOC_ADC_DIGI_DATA_BYTES_PER_CONV*conf->convCnt, ///< 转换完成后，DMA会把数据放入缓冲区，这个配置就是该缓冲区的大小。
        .conv_frame_size = SOC_ADC_DIGI_DATA_BYTES_PER_CONV*conf->convCnt,    ///< 一个ADC数据帧的大小。也即用户调用read函数一次读取的大小。
        .flags = {.flush_pool = true},  ///< 缓冲区满了的时候，要不要清空
    };
    ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config, handle));

    // step3: 配置ADC转换器的基本参数
    ESP_LOGI(TAG,"开始配置ADC外设的批量转换模式");
    adc_continuous_config_t dig_cfg = {
        //.pattern_num =    (uint32_t)conf->channelCnt,    // 总共多少个通道
        .adc_pattern =    adc_pattern,                   // 每个通道的转换配置
        .sample_freq_hz = SOC_ADC_SAMPLE_FREQ_THRES_LOW, // 最慢频率
        .conv_mode =      conf->convMode,                // 转换策略
        .format =         ADC_DIGI_OUTPUT_FORMAT_TYPE2,  // 12位输出结果
    };
    ESP_ERROR_CHECK(adc_continuous_config((*handle), &dig_cfg));

    // step4：配置ADC转换器的回调函数，用于手动滤波
    ESP_LOGW(TAG,"开始注册ADC外设批量转换的回调函数");
    if (conf->cb.on_conv_done != NULL || conf->cb.on_pool_ovf != NULL)
    ESP_ERROR_CHECK(adc_continuous_register_event_callbacks(*handle, &conf->cb, NULL));

    ESP_LOGI(TAG,"初始化成功");
    return (MyAdc_t)handle;
}

/**
 * @brief 删除ADC对象
 * 
 * @param handle 
 */
void MyAdcHandleDelete(MyAdc_t handle)
{
    free(handle);
}

/**
 * @brief 启动ADC对象
 * 
 * @param handle 
 * @return esp_err_t 
 */
esp_err_t MyAdcHandleStart(MyAdc_t handle)
{
    esp_err_t err = adc_continuous_start(*handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG,"ADC批量转换任务开启失败，错误码%d",err);
    }
    return err;
}

/**
 * @brief 停止ADC对象
 * 
 * @param handle 
 * @return esp_err_t 
 */
esp_err_t MyAdcHandleStop(MyAdc_t handle)
{
    esp_err_t err = adc_continuous_stop(*handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG,"ADC批量转换任务关闭失败，错误码%d",err);
    }
    return ESP_OK;
}

/**
 * @brief 创建ADC转换数据
 * 
 * @param conf 
 * @return MyAdcData_t 
 */
MyAdcData_t MyAdcDataCreate(const MyAdcConfig_t *conf)
{
    ESP_LOGW(TAG,"开始创建ADC批量数据句柄");
    MyAdcData_t data = (MyAdcData_t)malloc(sizeof(MyAdcDataStatic_Handle));
    if (data == NULL){
        ESP_LOGE(TAG,"空间不足，无法申请ADC批量数据");
        return NULL;
    }

    data->channelArr = malloc(sizeof(adc_channel_t)*conf->taskCnt);
    //data->periArr = malloc(sizeof(adc_unit_t)*conf->taskCnt);
    data->voltArr = malloc(sizeof(float)*conf->taskCnt);
    data->prv_buffer = malloc(SOC_ADC_DIGI_DATA_BYTES_PER_CONV*conf->convCnt);

    data->taskCnt = conf->taskCnt;
    // if (data->channelArr == NULL || data->periArr == NULL || data->voltArr == NULL || data->prv_buffer) {
    //     ESP_LOGE(TAG,"空间不足，无法申请ADC批量数据");
    //     free(data->channelArr);
    //     //free(data->periArr);
    //     free(data->voltArr);
    //     free(data->prv_buffer);
    //     free(data);
    //     return NULL;
    // }
    return data;
}

/**
 * @brief 删除ADC转换数据
 * 
 * @param handle 
 */
void myAdcDataDelete(MyAdcData_t handle)
{
    free(handle->channelArr);
    //free(handle->periArr);
    free(handle->voltArr);
    free(handle->prv_buffer);
    free(handle);
}

/**
 * @brief 获取ADC批量转换过的数据，滤波后得到单个数据用于输出
 * 
 * @param[IN] handle 
 * @param[OUT] data 
 * @return esp_err_t 
 */
// esp_err_t MyAdcDataGetMean(myAdc_t handle,MyAdcData_t data)
// {
//     size_t real_size;
//     esp_err_t err = adc_continuous_read(*handle,data->prv_buffer,SOC_ADC_DIGI_RESULT_BYTES*data->taskCnt,&real_size,0);

// }



// void adc_test(void)
// {
//     // 初始化ADC外设并获取句柄
//     adc_continuous_handle_t handle = NULL;
//     continuous_adc_init(channel, sizeof(channel) / sizeof(adc_channel_t), &handle);
//     // 启动ADC转换
//     ESP_ERROR_CHECK(adc_continuous_start(handle));

//     while (1) {
//         vTaskDelay(100);
//         for (int i = 0; i < 256; i += SOC_ADC_DIGI_RESULT_BYTES) {
//             adc_digi_output_data_t *p = (adc_digi_output_data_t*)&adcData[i];
//             uint32_t chan_num = EXAMPLE_ADC_GET_CHANNEL(p);
//             uint32_t data = EXAMPLE_ADC_GET_DATA(p);
//             printf("chn=%ld,data=%ld\n",chan_num,data);
//         }
//     }

// }
