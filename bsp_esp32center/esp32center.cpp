#include <esp_log.h>
#include <driver/uart.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#include "rdlc.h"
#include "esp32center.h"

static const char *TAG = "Board-Center";
#define BOARD_CENTER_EVENT_TX_DONE (1 << 0)

static Rdlc_t g_proto;
static QueueHandle_t uart_queue;
static uint8_t *g_txFrame;
static size_t   g_txFrameSize;
static EventGroupHandle_t g_commEvent;
static RdlcOnParse_fptr *g_rxCallback;
static uint8_t g_rxCallbackCount;
static uint8_t g_rxCallbackMax;
static uint8_t g_rxFreq;

static int BoardCenterRdlcRxCallback(Rdlc_t handle,RdlcAddr_t addr,const uint8_t* data,uint16_t size);
static int BoardCenterRdlcVprintf(RdlcLogLevel_t level,const char *fmt,va_list args);
static void BoardCenterUartRxTask(void *args);

/**
 * @brief 初始化ESP32核心板的基本功能
 * 
 * @param conf 
 * @return esp_err_t 
 */
esp_err_t BoardCenterCtrl_Init(const CenterConf_t *conf)
{
    // 创建接收任务
    g_rxFreq = conf->uartRxTaskFreq;
    ESP_ERROR_CHECK(xTaskCreate(BoardCenterUartRxTask,NULL,1024,NULL,conf->uartRxTaskPrio,NULL));
    
    // 配置串口协议
    RdlcConfig_t g_protoConfig = {
        .msgMaxSize = conf->rdlcMsgSize,
        .msgMaxEscapeSize = conf->rdlcMsgSize,
        .cbParsed = BoardCenterRdlcRxCallback,
        .cbError = NULL,
    };
    RdlcPort_t g_protoPort = {
        .portMalloc = malloc,
        .portFree = free,
        .portPrintf = BoardCenterRdlcVprintf,
    };
    g_proto = xRdlcCreate(&g_protoConfig,&g_protoPort);
    ESP_ERROR_CHECK((int)g_proto);
    //vRdlcSetLogLevel(g_proto,RDLC_LOG_DEBUG);

    // 配置串口
    uart_config_t uart_config = {
        .baud_rate = (int)conf->rdlcBaud,	//波特率
        .data_bits = UART_DATA_8_BITS,	//数据位
        .parity = UART_PARITY_DISABLE,	//奇偶校验
        .stop_bits = UART_STOP_BITS_1,	//停止位
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,	//流控
    };
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_1, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_1, 23, 22, -1, -1));
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_1, 512,512, 10, &uart_queue, 0));

    // 申请接收回调函数
    g_rxCallback = (RdlcOnParse_fptr *)malloc(sizeof(RdlcOnParse_fptr) * conf->rdlcCallbackMax);
    ESP_ERROR_CHECK((int)g_rxCallback);
    g_rxCallbackCount = 0;
    g_rxCallbackMax = conf->rdlcCallbackMax;

    // 申请发送缓冲区
    g_txFrameSize = RDLC_GET_FRAME_SIZE(conf->rdlcMsgSize,conf->rdlcMsgSize);
    g_txFrame = (uint8_t *)malloc(g_txFrameSize);
    if (g_txFrame == NULL) {
        ESP_LOGE(TAG,"空间不足，初始化失败");
        return ESP_ERR_NO_MEM;
    }

    // 注册事件组
    g_commEvent = xEventGroupCreate();
    if (g_commEvent == NULL) {
        ESP_LOGE(TAG,"空间不足，初始化失败");
        return ESP_ERR_NO_MEM;
    }
    return ESP_OK;
}

/**
 * @brief 向ESP32核心板上的CH32发送RDLC帧
 * 
 * @param addr 
 * @param data 
 * @param size 
 * @param tickToWait 
 * @return esp_err_t 
 */
esp_err_t BoardCenterCtrl_Ch32Send(RdlcAddr_t addr,const void *data,size_t size,int tickToWait)
{
    if (g_proto == NULL) {
        ESP_LOGE(TAG,"尚未初始化");
        return ESP_ERR_NOT_ALLOWED;
    }
    if (g_commEvent) {
        xEventGroupWaitBits(g_commEvent,BOARD_CENTER_EVENT_TX_DONE,pdTRUE,pdFALSE,tickToWait);
    }
    int len = xRdlcWriteBytes(g_proto,addr,(const uint8_t*)data,size,g_txFrame,g_txFrameSize);
    if (len < RDLC_OK) {
        ESP_LOGE(TAG,"RDLC封包失败，错误码%d",len);
        xEventGroupSetBits(g_commEvent,BOARD_CENTER_EVENT_TX_DONE);
        return ESP_FAIL;
    }
    int byteSend = uart_write_bytes(UART_NUM_1,g_txFrame,len);
    if (byteSend != len) {
        ESP_LOGE(TAG,"UART发送失败或未能发送完整帧，发送总计%d",byteSend);
        xEventGroupSetBits(g_commEvent,BOARD_CENTER_EVENT_TX_DONE);
        return ESP_FAIL;
    }
    xEventGroupSetBits(g_commEvent,BOARD_CENTER_EVENT_TX_DONE);
    return ESP_OK;
}

/**
 * @brief 事件驱动的RDLC接收任务
 * 
 * @param args 
 */
void BoardCenterUartRxTask(void *args)
{
    static uint8_t data[100];
    while(1) {
        int len = uart_read_bytes(UART_NUM_1, data, 100, g_rxFreq);
        xRdlcReadBytes(g_proto,data,len);
    }
}
/**
 * @brief 向与CH32的UART通信中注册接收回调函数
 * 
 * @param callback 
 * @return esp_err_t 
 */
esp_err_t BoardCenterCtrl_Ch32RegisterRxCallback(RdlcOnParse_fptr callback)
{
    if (g_proto == NULL) {
        ESP_LOGE(TAG,"尚未初始化");
        return ESP_ERR_NOT_ALLOWED;
    }
    if(g_rxCallbackCount >= g_rxCallbackMax) {
        ESP_LOGE(TAG,"回调函数队列已满");
        return ESP_ERR_NO_MEM;
    }
    g_rxCallback[g_rxCallbackCount] = callback;
    return ESP_OK;
}
/**
 * @brief RDLC协议接收回调函数
 * 
 * @param handle 
 * @param addr 
 * @param data 
 * @param size 
 * @return int 
 */
static int BoardCenterRdlcRxCallback(Rdlc_t handle,RdlcAddr_t addr,const uint8_t* data,uint16_t size)
{
    for(int i=0;i<g_rxCallbackCount;i++)
        g_rxCallback[i](handle,addr,data,size);
    return RDLC_OK;
}
/**
 * @brief RDLC协议日志打印
 * 
 * @param level 
 * @param fmt 
 * @param args 
 * @return int 
 */
static int BoardCenterRdlcVprintf(RdlcLogLevel_t level,const char *fmt,va_list args)
{
    printf("[%d] ",level);
    int len = vprintf(fmt,args);
    printf("\n");
    return len;
}