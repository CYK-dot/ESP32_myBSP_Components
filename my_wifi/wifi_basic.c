#include "my_wifi.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif_net_stack.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#if IP_NAPT
#include "lwip/lwip_napt.h"
#endif
#include "lwip/err.h"
#include "lwip/sys.h"

#if CONFIG_ESP_WIFI_AUTH_OPEN
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD   WIFI_AUTH_OPEN
#elif CONFIG_ESP_WIFI_AUTH_WEP
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD   WIFI_AUTH_WEP
#elif CONFIG_ESP_WIFI_AUTH_WPA_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD   WIFI_AUTH_WPA_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD   WIFI_AUTH_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD   WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD   WIFI_AUTH_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD   WIFI_AUTH_WPA2_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WAPI_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD   WIFI_AUTH_WAPI_PSK
#endif


#define WIFI_CONNECTED_BIT BIT0        ///< STA模式下，接入AP
#define WIFI_FAIL_BIT      BIT1        ///< STA模式下，尝试接入次数超过预期
#define WIFI_STA_READY_BIT BIT2        ///< STA外设准备完毕
#define WIFI_AP_CONNECTED_BIT BIT3     ///< AP模式下，STA接入

#define DHCPS_OFFER_DNS             0x02

static const char *TAG_AP = "MyWifi-AP";
static const char *TAG_STA = "MyWifi-STA";
static const char *TAG_COMMON = "MyWifi-COM";

static esp_netif_t *esp_netif_sta = NULL;
static esp_netif_t *esp_netif_ap = NULL;

static int s_retry_num = 0;
static EventGroupHandle_t s_wifi_event_group;

static void wifi_event_handler(void *arg, esp_event_base_t event_base,int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *) event_data;
        ESP_LOGI(TAG_AP, "设备 "MACSTR" 接入WIFI, AID=%d",MAC2STR(event->mac), event->aid);
        xEventGroupSetBits(s_wifi_event_group, WIFI_AP_CONNECTED_BIT);
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *) event_data;
        ESP_LOGI(TAG_AP, "设备 "MACSTR" 断开WIFI, AID=%d, 原因:%d",MAC2STR(event->mac), event->aid, event->reason);
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG_STA, "STA初始化完成，等待接入热点");
        xEventGroupSetBits(s_wifi_event_group, WIFI_STA_READY_BIT);
    } 
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
        ESP_LOGI(TAG_STA, "取得IP:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_AP_STAIPASSIGNED) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
        ESP_LOGI(TAG_AP,"成功为STA赋予一个IP:" IPSTR,IP2STR(&event->ip_info.ip));
        // 未知原因，次事件不可用
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        s_retry_num ++;
        ESP_LOGE(TAG_STA,"断开与AP的连接");
        esp_wifi_connect();
    }
}
static esp_netif_t *wifi_init_softap(const wifi_config_t *wifi_ap_config)
{
    esp_netif_t *esp_netif_ap = esp_netif_create_default_wifi_ap();
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, (wifi_config_t*)wifi_ap_config));
    ESP_LOGI(TAG_AP, "WIFI热点初始化完成. 名称:%s 密码:%s 信道:%d",wifi_ap_config->ap.ssid, wifi_ap_config->ap.password, wifi_ap_config->ap.channel);
    return esp_netif_ap;
}
static esp_netif_t *wifi_init_sta(const wifi_config_t *wifi_sta_config)
{
    esp_netif_t *esp_netif_sta = esp_netif_create_default_wifi_sta();
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, (wifi_config_t*)wifi_sta_config));
    ESP_LOGI(TAG_STA, "WIFI接入点初始化完成. 名称:%s 密码:%s 信道全扫描",wifi_sta_config->sta.ssid, wifi_sta_config->sta.password);
    return esp_netif_sta;
}
static void softap_set_dns_addr(esp_netif_t *esp_netif_ap,esp_netif_t *esp_netif_sta)
{
    esp_netif_dns_info_t dns;
    esp_netif_get_dns_info(esp_netif_sta,ESP_NETIF_DNS_MAIN,&dns);
    uint8_t dhcps_offer_option = DHCPS_OFFER_DNS;
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_netif_dhcps_stop(esp_netif_ap));
    ESP_ERROR_CHECK(esp_netif_dhcps_option(esp_netif_ap, ESP_NETIF_OP_SET, ESP_NETIF_DOMAIN_NAME_SERVER, &dhcps_offer_option, sizeof(dhcps_offer_option)));
    ESP_ERROR_CHECK(esp_netif_set_dns_info(esp_netif_ap, ESP_NETIF_DNS_MAIN, &dns));
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_netif_dhcps_start(esp_netif_ap));
}


/**
 * @brief 完整扫描周边存在的wifi热点，打印信息并保存到占用数组，然后返回占用最少的信道
 * 
 * @param scanListSize  需要扫描多少个热点，不要给太大，不然会爆堆栈
 * @param channelOccupy 信道情况统计，数字越大占有者越强，对于802.11a/b/n，本数组长度不应小于13
 * @return int
 * 
 * @note 本函数纯图一乐，请使用专业的软件/工具评估信道情况 
 */
int MyWifiScan80211n(int scanListSize,uint32_t *channelOccupy)
{
    #define CHANNEL_80211_MAX 13
    ESP_LOGW(TAG_STA,"信道扫描$ 初始化Flash");
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    ESP_LOGW(TAG_STA,"信道扫描$ 初始化wifi");
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_LOGW(TAG_STA,"信道扫描$ 准备扫描");
    uint16_t number = scanListSize;
    wifi_ap_record_t ap_info[scanListSize];
    uint16_t ap_count = 0;
    memset(ap_info, 0, sizeof(ap_info));
    for(int i=0;i<=13;i++){
        channelOccupy[i]=0;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_scan_start(NULL, true));

    ESP_LOGI(TAG_STA, "信道扫描结果$ 最多有 %u 个数据", number);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
    ESP_LOGI(TAG_STA, "信道扫描结果$ 共扫描到 %u 个热点",ap_count);
    for (int i = 0; i < number; i++) {
        ESP_LOGI(TAG_STA, "热点名称=\t\t%s 信号强度=\t\t%d 信道=\t\t%d", ap_info[i].ssid,ap_info[i].rssi,ap_info[i].primary);
        channelOccupy[ap_info[i].primary]++;
    }

    int minCount = INT_MAX;
    int minIndex = 0;
    for(int i=1;i <= CHANNEL_80211_MAX; i++) {
        if (channelOccupy[i] < minCount){
            minCount = channelOccupy[i];
            minIndex = i;
        }
    }
    ESP_LOGW(TAG_STA, "信道扫描$ 信道%d是最优信道，只有%d个人占用",minIndex,minCount);
    ESP_LOGW(TAG_STA, "信道扫描$ 关闭wifi并清理数据");
    ESP_ERROR_CHECK(esp_wifi_clear_ap_list());
    ESP_ERROR_CHECK(esp_wifi_stop());
    return ESP_OK;
    #undef CHANNEL_80211_MAX
}

/**
 * @brief 获取默认的配置参数
 * 
 * @param ssid wifi名称
 * @param password wifi密码
 * @param mode wifi模式，仅应填入WIFI_MODE_AP/WIFI_MODE_STA
 * @param config 需要被配置的结构体
 * 
 * @note 当配置AP模式时，需要手动提供一个信道，最好是干扰最小的
 * @return esp_err_t 
 */
esp_err_t MyWifiSetConfigDefault(const char *ssid,const char *password,
    size_t ssid_len,size_t password_len,
    wifi_mode_t mode,wifi_config_t *config)
{
    if (mode == WIFI_MODE_STA){
        wifi_config_t sta_cfg = {
            .sta= {
                .scan_method = WIFI_ALL_CHANNEL_SCAN,
                .failure_retry_cnt = 20,
                .threshold.authmode = WIFI_AUTH_WPA_WPA2_PSK,
                .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
            }
        };
        strncpy((char *)sta_cfg.sta.ssid, ssid, ssid_len);
        strncpy((char *)sta_cfg.sta.password, password, password_len);
        config->sta = sta_cfg.sta;
        ESP_LOGW(TAG_COMMON,"获取默认配置$ 接入热点名称=%s,密码=%s",config->sta.ssid,config->sta.password);
    }

    else if (mode == WIFI_MODE_AP){
        wifi_config_t ap_cfg = {
            .ap = {
                .channel = 1,
                .max_connection = CONFIG_ESP_MAX_STA_CONN_AP,
                .authmode = WIFI_AUTH_WPA2_PSK,
                .pmf_cfg = {
                    .required = false,
                },
            },
        };
        strncpy((char *)ap_cfg.ap.ssid, ssid, ssid_len);
        ap_cfg.ap.ssid_len = strlen((char*)ap_cfg.ap.ssid);
        strncpy((char *)ap_cfg.ap.password, password, password_len);
        config->ap = ap_cfg.ap;
        ESP_LOGW(TAG_COMMON,"获取默认配置$ 产生热点名称=%s,密码=%s",config->ap.ssid,config->ap.password);
    }

    else{
        ESP_LOGE(TAG_COMMON,"不能获取APSTA模式的默认参数，请分开获取");
        ESP_ERROR_CHECK(0);
        return ESP_ERR_INVALID_ARG;
    }
    return ESP_OK;
}

/**
 * @brief 设置热点的信道
 * 
 * @param config 需要配置的结构体
 * @param channel 信道，1~11。推荐1、6、11
 * @return esp_err_t 
 */
esp_err_t MyWifiSetConfigChannel(int channel,wifi_config_t *config)
{
    config->ap.channel = channel;
    return ESP_OK;
}

/**
 * @brief 配置wifi的名称、密码和信道，并启动
 * 
 * @param apConfig 带有配置的wifi参数(作为热点)
 * @param staConfig 带有配置的wifi参数(接入别人)
 * @param mode wifi模式，可以填入WIFI_MODE_AP/WIFI_MODE_STA/WIFI_MODE_APSTA
 * @return esp_err_t 
 * 
 * @warning 易错：AP和STA并不对等，因为函数形参的位置不一样，写错了会死在wifi_init_softap/wifi_init_sta的ESP_ERROR_CHECK中
 */
esp_err_t MyWifiSetup(const wifi_config_t *apConfig,const wifi_config_t *staConfig,wifi_mode_t mode)
{
    ESP_LOGW(TAG_COMMON,"初始化开始$ 模式=%d",mode);

    ESP_LOGW(TAG_COMMON,"初始化协议栈$");
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_LOGW(TAG_COMMON,"初始化Flash驱动$");
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_LOGW(TAG_COMMON,"初始化wifi回调函数$");
    s_wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                    ESP_EVENT_ANY_ID,
                    &wifi_event_handler,
                    NULL,
                    NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                    IP_EVENT_STA_GOT_IP,
                    &wifi_event_handler,
                    NULL,
                    NULL));

    ESP_LOGW(TAG_COMMON,"初始化wifi$");
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(mode));


    ESP_LOGW(TAG_COMMON,"中国允许1~13信道$");
    wifi_country_t cn_country = {
        .cc = "CN",
        .schan = 1,
        .nchan = 13,
        .policy = WIFI_COUNTRY_POLICY_MANUAL
    };
    ESP_ERROR_CHECK(esp_wifi_set_country(&cn_country));

    ESP_LOGW(TAG_COMMON,"配置wifi模式$");
    if (mode == WIFI_MODE_AP){
        ESP_LOGW(TAG_COMMON,"初始化wifi-AP$");
        esp_netif_ap = wifi_init_softap(apConfig);
    }
    else if (mode == WIFI_MODE_STA){
        ESP_LOGW(TAG_COMMON,"初始化wifi-STA$");
        esp_netif_sta = wifi_init_sta(staConfig);
    }
    else if (mode == WIFI_MODE_APSTA){
        ESP_LOGW(TAG_COMMON,"初始化wifi-APSTA$");
        esp_netif_ap = wifi_init_softap(apConfig);    
        esp_netif_sta = wifi_init_sta(staConfig);
    }
    else{
        ESP_LOGE(TAG_COMMON,"不允许的WIFI模式，请在AP/STA中选择其中1~2个");
        return ESP_ERR_INVALID_ARG;
    }
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGW(TAG_COMMON,"wifi配置完成$");
    return ESP_OK;
}

/**
 * @brief 完成链路层及以上的配置后，调用本函数让物理层开始建立连接
 * 
 */
esp_err_t MyWifiWaitConnection(wifi_mode_t mode)
{
    ESP_LOGW(TAG_COMMON,"开始联网，联网模式为%d",mode);

    // STA模式联网
    if (mode == WIFI_MODE_STA) {
        ESP_LOGI(TAG_STA,"等待STA外设准备完毕");
        xEventGroupWaitBits(s_wifi_event_group,WIFI_STA_READY_BIT,pdFALSE,pdFALSE,portMAX_DELAY);

        ESP_LOGI(TAG_STA,"等待STA接入热点");
        esp_wifi_connect();
        EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);
        if (bits & WIFI_CONNECTED_BIT) {
            ESP_LOGI(TAG_STA,"STA联网成功，可以开始使用链路层及以上应用");
            return ESP_OK;
        }
        if (bits & WIFI_FAIL_BIT) {
            ESP_LOGE(TAG_STA,"无法连接指定热点，超过最大尝试次数");
            return ESP_FAIL;
        }
    }

    // AP模式联网
    else if (mode == WIFI_MODE_AP) {
        ESP_LOGI(TAG_AP,"等待1个STA设备接入本热点");
        xEventGroupWaitBits(s_wifi_event_group,WIFI_AP_CONNECTED_BIT,pdTRUE,pdFALSE,portMAX_DELAY);
        vTaskDelay(50); //暂时用死等的方式等待分配IP地址
        ESP_LOGI(TAG_AP,"AP联网成功，可以开始使用链路层及以上应用");
        return ESP_OK;
    }
    
    // APSTA模式联网
    else if (mode == WIFI_MODE_APSTA){
        ESP_LOGI(TAG_AP,"等待1个STA设备接入本热点");
        xEventGroupWaitBits(s_wifi_event_group,WIFI_AP_CONNECTED_BIT,pdTRUE,pdFALSE,portMAX_DELAY);

        ESP_LOGI(TAG_STA,"等待STA外设准备完毕");
        xEventGroupWaitBits(s_wifi_event_group,WIFI_STA_READY_BIT,pdFALSE,pdFALSE,portMAX_DELAY);

        ESP_LOGI(TAG_STA,"等待STA接入热点");
        EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                               WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                               pdFALSE,
                                               pdFALSE,
                                               portMAX_DELAY);
        if (bits & WIFI_CONNECTED_BIT) {
            ESP_LOGI(TAG_STA, "STA联网成功，开始配置AP-STA的DNS服务器");
            softap_set_dns_addr(esp_netif_ap,esp_netif_sta);
        } 
        else if (bits & WIFI_FAIL_BIT) {
            ESP_LOGE(TAG_STA,"无法连接指定热点，超过最大尝试次数");
            return ESP_FAIL;
        } 
        else {
            ESP_LOGE(TAG_STA, "未知事件");
        }

        ESP_LOGI(TAG_COMMON,"开始配置AP-STA的NAPT数据转发");
        esp_netif_set_default_netif(esp_netif_sta);
        if (esp_netif_napt_enable(esp_netif_ap) != ESP_OK) {
            ESP_LOGE(TAG_COMMON, "NAPT在适配器上未开启: %p", esp_netif_ap);
        }

        ESP_LOGI(TAG_AP,"APSTA联网成功，可以开始使用链路层及以上应用");
        return ESP_OK;
    }
    return ESP_ERR_NOT_ALLOWED;
}

/**
 * @brief 获取当前wifi断连的次数
 * 
 * @return int 
 */
int MyWifiGetStaConnectCount(void)
{
    return s_retry_num;
}