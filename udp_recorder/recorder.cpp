#include <esp_log.h>
#include <esp_err.h>
#include <lwip/err.h>
#include <lwip/sys.h>
#include <lwip/sockets.h>
#include <driver/uart.h>

#include "rdlc.h"
#include "my_wifi.h"
#include "udp_recorder.h"

const char *TAG = "UDP-Recorder";



static QueueHandle_t uart_queue;
static Rdlc_t uartProto;
static int sockfd; 
static socklen_t addr_len;
static struct sockaddr_in server_addr;
static struct sockaddr_in client_addr;  

/**
 * @brief 串口协议解析回调函数
 * 
 * @param handle 
 * @param data 
 * @param size 
 * @return int 
 */
static int UdpUartParseCallback(Rdlc_t handle,const uint8_t* data,uint16_t size)
{
    ESP_LOGE(TAG,"成功解析出%hd个数据",size);
    addr_len = sizeof(client_addr);
    if (sendto(sockfd, data, sizeof(size), 0, (struct sockaddr *)&client_addr, addr_len) < 0) {
        perror("Sendto failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    return RDLC_OK;
}

static int  UdpUartPrintfPort(RdlcLogLevel_t level,const char *fmt,va_list args)
{
    printf("[%d] ",level);
    int len = vprintf(fmt,args);
    printf("\n");
    return len;
}

/**
 * @brief 初始化UDP日志记录器
 * 
 * @return esp_err_t 
 */
esp_err_t UdpRecorderInit(void)
{
    // 配置wifi外设
    static wifi_config_t ap;
    MyWifiSetConfigDefault("ESP32(wifi6)","12345678",strlen("ESP32(wifi6)"),strlen("12345678"),WIFI_MODE_AP,&ap);
    MyWifiSetup(&ap,NULL,WIFI_MODE_AP);
    // 配置物理层协议
    MyWifiSetProtocol(WIFI_IF_AP,WIFI_BW40,WIFI_PTL_80211_N);
    // 联网
    MyWifiWaitConnection(WIFI_MODE_AP);

    // 配置UDP服务器
    #define SERVER_PORT 3333      // 服务器的端口号
    #define CLIENT_PORT 3333      // 客户端的端口号
    #define SERVER_IP "192.168.4.1"  // 服务器IP地址
    #define CLIENT_IP "192.168.4.2"  // 客户端IP地址

    // 创建UDP套接字
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 设置服务器地址
    memset(&server_addr, 0, sizeof(server_addr));  // 清空结构体
    server_addr.sin_family = AF_INET;               // 地址簇设置为IPv4
    server_addr.sin_port = htons(SERVER_PORT);      // 设置服务器端口
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);  // 设置服务器IP地址

    // 绑定套接字到指定的服务器地址
    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    // 设置客户端地址
    memset(&client_addr, 0, sizeof(client_addr));  // 清空结构体
    client_addr.sin_family = AF_INET;               // 地址簇设置为IPv4
    client_addr.sin_port = htons(CLIENT_PORT);      // 设置客户端端口号
    client_addr.sin_addr.s_addr = inet_addr(CLIENT_IP);  // 设置客户端IP地址
    ESP_LOGI(TAG,"UDP日志服务器已经启动");

    // 配置串口
    uart_config_t uart_config = {
        .baud_rate = 230400,			//波特率
        .data_bits = UART_DATA_8_BITS,	//数据位
        .parity = UART_PARITY_DISABLE,	//奇偶校验
        .stop_bits = UART_STOP_BITS_1,	//停止位
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,	//流控
    };
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_1, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_1, 6, 7, -1, -1));
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_1, 512,512, 10, &uart_queue, 0));
    return ESP_OK;
}

/**
 * @brief 测试初始化
 * 
 */
void UdpRecorderTestInit(void)
{
    UdpRecorderInit();
}

/**
 * @brief 测试主函数
 * 
 */
void UdpRecorderTestMain(void)
{
    static uint8_t data[200];
    static uint8_t txBuffer[200];
    int len;

    //len = xRdlcWriteBytes(uartProto,(const uint8_t*)"Hello?",sizeof("Hello?"),txBuffer,sizeof(txBuffer));
    //uart_write_bytes(UART_NUM_1, (const char*)txBuffer, len);
    //ESP_LOGW(TAG,"数据开始发送");

    vTaskDelay(400);

    len = uart_read_bytes(UART_NUM_1, data, 100, 200);//20ms处理一次
    ESP_LOGI(TAG,"收到%d个数据",len);

    xRdlcReadBytes(uartProto,data,len);
}

/**
 * @brief 正常工作的函数，放在while(1)中，并且无需延时
 * @note 无论是否测试，此函数无需改动。
 * 
 */
void UdpRecorderMain(void)
{
    static uint8_t data[512];
    int len;
    len = uart_read_bytes(UART_NUM_1, data, 100, 200);//20ms处理一次
    ESP_LOGI(TAG,"收到%d个数据，前1个为%#hhX",len,data[0]);
    xRdlcReadBytes(uartProto,data,len);
}

