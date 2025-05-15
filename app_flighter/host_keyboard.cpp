#include <string.h>
#include <driver/uart.h>
#include <driver/gpio.h>
#include <esp_log.h>

#include "rdlc.h"
#include "flighter_host.h"

const char *TAG = "Flighter-Host";
const RdlcAddr_t g_protoAddr = {.srcAddr = 0x01,.dstAddr = 0x01};

static Rdlc_t g_proto;
static QueueHandle_t uart_queue;
static FlighterKeyboard_t g_keyboard;

/**
 * @brief 协议回调函数
 * 
 * @param handle 
 * @param addr 
 * @param data 
 * @param size 
 * @return int 
 */
static int FlighterUartParseCallback(Rdlc_t handle,RdlcAddr_t addr,const uint8_t* data,uint16_t size)
{
    uint16_t keyLxRaw = ((uint16_t)data[15])<<8 | ((uint16_t)data[14]);
    uint16_t keyLyRaw = ((uint16_t)data[11])<<8 | ((uint16_t)data[10]);
    uint16_t keyRxRaw = ((uint16_t)data[13])<<8 | ((uint16_t)data[12]);
    uint16_t keyRyRaw = ((uint16_t)data[9])<<8 | ((uint16_t)data[8]);
    uint16_t liBattery= ((uint16_t)data[3])<<8 | ((uint16_t)data[2]);

    g_keyboard.rockLX = ((float)(keyLxRaw-270))/540.0f;
    g_keyboard.rockLY = -((float)(keyLyRaw-270))/540.0f;
    g_keyboard.rockRX = ((float)(keyRxRaw-270))/540.0f;
    g_keyboard.rockRY = -((float)(keyRyRaw-270))/540.0f;
    //ESP_LOGI(TAG,"收到摇杆：%hd,%hd,%hd,%hd",keyLxRaw,keyLyRaw,keyRxRaw,keyRyRaw);
    ESP_LOGI(TAG,"收到摇杆：%.1f %.1f %.1f %.1f",g_keyboard.rockLX,g_keyboard.rockLY,g_keyboard.rockRX,g_keyboard.rockRY);
    //ESP_LOGI(TAG,"按键=%d",gpio_get_level(GPIO_NUM_16));
    return RDLC_OK;
}
/**
 * @brief 协议日志接口
 * 
 * @param level 
 * @param fmt 
 * @param args 
 * @return int 
 */
static int FlighterUartPrintfPort(RdlcLogLevel_t level,const char *fmt,va_list args)
{
    printf("[%d] ",level);
    int len = vprintf(fmt,args);
    printf("\n");
    return len;
}
/**
 * @brief 初始化遥控器键盘
 * 
 */
void FlighterHostSetup(void)
{
    // 配置串口协议
    RdlcConfig_t g_protoConfig = {
        .msgMaxSize = 16,
        .msgMaxEscapeSize = 16,
        .cbParsed = FlighterUartParseCallback,
        .cbError = NULL,
    };
    RdlcPort_t g_protoPort = {
        .portMalloc = malloc,
        .portFree = free,
        .portPrintf = FlighterUartPrintfPort,
    };
    g_proto = xRdlcCreate(&g_protoConfig,&g_protoPort);
    //vRdlcSetLogLevel(g_proto,RDLC_LOG_DEBUG);

    // 配置串口
    uart_config_t uart_config = {
        .baud_rate = 230400,			//波特率
        .data_bits = UART_DATA_8_BITS,	//数据位
        .parity = UART_PARITY_DISABLE,	//奇偶校验
        .stop_bits = UART_STOP_BITS_1,	//停止位
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,	//流控
    };
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_1, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_1, -1, 22, -1, -1));
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_1, 512,512, 10, &uart_queue, 0));
}

/**
 * @brief 更新键盘信息
 * 
 * @param output 
 */
void FlighterHostKeyboardUpdate(uint16_t tickToWait,FlighterKeyboard_t* output)
{
    static uint8_t data[200];
    int len = uart_read_bytes(UART_NUM_1, data, 100, tickToWait);
    //ESP_LOGI(TAG,"收到%d个数据",len);
    xRdlcReadBytes(g_proto,data,len);
    memcpy(output,&g_keyboard,sizeof(FlighterKeyboard_t));
}



