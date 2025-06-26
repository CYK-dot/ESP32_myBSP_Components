/**
 * @file ewifi_basic.c
 * @author CYK-Dot
 * @brief 简易WiFi管理
 * @version 0.1
 * @date 2025-06-24
 *
 * @copyright Copyright (c) 2025
 */

/* 头文件引入 -----------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>

#include <esp_err.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <esp_mac.h>
#include <nvs_flash.h>

#include <lwip/inet.h>
#include <lwip/netdb.h>
#include <lwip/sockets.h>


#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/event_groups.h>

#include "easy_nvs.h"
#include "prv_ewifi.h"

#include "ewifi_basic.h"

/* 私有类型定义 ---------------------------------------------------------------------*/

/* 私有宏定义 -----------------------------------------------------------------------*/

static const char *TAG = "ewifi_basic";

#define DHCPS_OFFER_DNS    0x02        ///< DHCP提供DNS

#define WIFI_CONNECTED_BIT BIT0        ///< STA模式下，接入AP
#define WIFI_FAIL_BIT      BIT1        ///< STA模式下，尝试接入次数超过预期
#define WIFI_STA_READY_BIT BIT2        ///< STA外设准备完毕
#define WIFI_AP_CONNECTED_BIT BIT3     ///< AP模式下，STA接入

/* 全局变量声明 ---------------------------------------------------------------------*/

// wifi回调相关
static int s_retry_num = 0;
static EventGroupHandle_t s_wifi_event_group;

// netif句柄
static esp_netif_t *esp_netif_sta = NULL;
static esp_netif_t *esp_netif_ap = NULL;

/* 私有函数声明 ---------------------------------------------------------------------*/

static void wifi_event_handler(void *arg, esp_event_base_t event_base,int32_t event_id, void *event_data);
static esp_netif_t *wifi_init_softap(const wifi_config_t *wifi_ap_config);
static esp_netif_t *wifi_init_sta(const wifi_config_t *wifi_sta_config);
static void softap_set_dns_addr(esp_netif_t *esp_netif_ap,esp_netif_t *esp_netif_sta);

/* 导出函数定义 ---------------------------------------------------------------------*/

/**
 * @brief 打印某个指定的配置
 * 
 * @param conf 
 */
void ewifi_basic_print_conf(ewifi_conf_t *conf)
{
    ESP_LOGW(TAG,"展示配置,如下所示");
    if (conf->peri.mode == WIFI_MODE_AP)
    {
        ESP_LOGI(TAG,"模式:AP");
        ESP_LOGI(TAG,"协议:%d,频宽:%d,主频段:%d,是否固定速率:%hhd,速率:%d",conf->ptl.protocol,conf->phy.bandwidth,conf->phy.primary_channel,conf->phy.enable_fixed_tx_rate,conf->phy.fixed_tx_rate.rate);
        ESP_LOGI(TAG,"SSID:%s,PSWD:%s",conf->peri.config.ap.ssid,conf->peri.config.ap.password);
        ESP_LOGI(TAG,"信道:%d,带宽:%d",conf->phy.primary_channel,conf->phy.bandwidth);
        ESP_LOGI(TAG,"最大连接数:%d,隐藏SSID:%d",conf->peri.config.ap.max_connection,conf->peri.config.ap.ssid_hidden);
    }
    else
    {
        ESP_LOGI(TAG,"模式:STA");
        ESP_LOGI(TAG,"SSID:%s,PSWD:%s",conf->peri.config.sta.ssid,conf->peri.config.sta.password);
    }
    
}
/**
 * @brief 从宏定义中获取默认的AP配置
 * 
 * @param conf 
 * @return esp_err_t 
 */
esp_err_t ewifi_basic_get_conf_from_default_ap(ewifi_conf_t *conf)
{
    // 外设配置
    conf->peri.mode = WIFI_MODE_AP;
    conf->peri.config.ap.authmode = EASY_WIFI_DEFAULT_AP_AUTH;
    conf->peri.config.ap.beacon_interval = EASY_WIFI_DEFAULT_AP_BEACON_INTERVAL;
    conf->peri.config.ap.channel = 1;
    conf->peri.config.ap.csa_count = EASY_WIFI_DEFAULT_AP_CSA_INTERVAL;
    conf->peri.config.ap.dtim_period = EASY_WIFI_DEFAULT_AP_DTIM_INTERVAL;
    conf->peri.config.ap.ftm_responder = false;
    conf->peri.config.ap.max_connection = EASY_WIFI_DEFAULT_AP_MAX_CONNECTION;
    conf->peri.config.ap.pairwise_cipher = EASY_WIFI_DEFAULT_AP_CIPHER;
    conf->peri.config.ap.pmf_cfg.capable = EASY_WIFI_DEFAULT_AP_PMF_CAPABLE;
    conf->peri.config.ap.pmf_cfg.required = EASY_WIFI_DEFAULT_AP_PMF_REQUIRE;
    conf->peri.config.ap.sae_pwe_h2e = EASY_WIFI_DEFAULT_AP_SAE;
    conf->peri.config.ap.transition_disable = EASY_WIFI_DEFAULT_AP_TRANSITION;
    conf->peri.config.ap.ssid_hidden = EASY_WIFI_DEFAULT_AP_SSID_HIDDEN;
    strcpy((char*)(conf->peri.config.ap.password), EASY_WIFI_DEFAULT_PASSWORD);
    strcpy((char*)(conf->peri.config.ap.ssid), EASY_WIFI_DEFAULT_SSID);
    conf->peri.config.ap.ssid_len = strlen(EASY_WIFI_DEFAULT_SSID);
    // 频段配置
    conf->phy.bandwidth = EASY_WIFI_DEFAULT_BANDWIDTH;
    conf->phy.primary_channel = EASY_WIFI_DEFAULT_AP_CHANNEL_P;
    conf->phy.secondary_channel = EASY_WIFI_DEFAULT_AP_CHANNEL_S;
    conf->phy.enable_fixed_tx_rate = false;
    // 协议配置
    conf->ptl.protocol = EASY_WIFI_DEFAULT_PTL;
    return ESP_OK;
}

/**
 * @brief 从宏定义中获取默认的STA配置
 * 
 * @param conf 
 * @return esp_err_t 
 */
esp_err_t ewifi_basic_get_conf_from_default_sta(ewifi_conf_t *conf)
{
    // 外设配置
    conf->peri.mode = WIFI_MODE_STA;
    conf->peri.config.sta.bssid_set = EASY_WIFI_DEFAULT_STA_BSSID_SET;
    conf->peri.config.sta.btm_enabled = EASY_WIFI_DEFAULT_STA_BTM;
    conf->peri.config.sta.channel = EASY_WIFI_DEFAULT_STA_CHANNEL;
    conf->peri.config.sta.failure_retry_cnt = EASY_WIFI_DEFAULT_STA_RETRY;
    conf->peri.config.sta.ft_enabled = EASY_WIFI_DEFAULT_STA_FT;
    conf->peri.config.sta.listen_interval = EASY_WIFI_DEFAULT_STA_INTERVAL;
    conf->peri.config.sta.mbo_enabled = EASY_WIFI_DEFAULT_STA_MBO;
    conf->peri.config.sta.owe_enabled = EASY_WIFI_DEFAULT_STA_OWE;
    conf->peri.config.sta.pmf_cfg.capable = EASY_WIFI_DEFAULT_STA_PMF_CAPABLE;
    conf->peri.config.sta.pmf_cfg.required = EASY_WIFI_DEFAULT_STA_PMF_REQUIRE;
    conf->peri.config.sta.rm_enabled = false;
    conf->peri.config.sta.sae_pk_mode = EASY_WIFI_DEFAULT_STA_SAE_PK_MODE;
    conf->peri.config.sta.sae_pwe_h2e = EASY_WIFI_DEFAULT_STA_SAE_PWE;
    conf->peri.config.sta.scan_method = EASY_WIFI_DEFAULT_STA_SCAN;
    conf->peri.config.sta.sort_method = EASY_WIFI_DEFAULT_STA_SORT_METHOD;
    conf->peri.config.sta.threshold.authmode = EASY_WIFI_DEFAULT_STA_AUTH;
    conf->peri.config.sta.threshold.rssi = EASY_WIFI_DEFAULT_STA_RSSI;
    conf->peri.config.sta.transition_disable = EASY_WIFI_DEFAULT_STA_TRANSITION;
    strcpy((char*)(conf->peri.config.sta.password), EASY_WIFI_DEFAULT_PASSWORD);
    strcpy((char*)(conf->peri.config.sta.ssid), EASY_WIFI_DEFAULT_SSID);
    // 频段配置
    conf->phy.bandwidth = EASY_WIFI_DEFAULT_BANDWIDTH;
    conf->phy.primary_channel = EASY_WIFI_DEFAULT_AP_CHANNEL_P;
    conf->phy.secondary_channel = EASY_WIFI_DEFAULT_AP_CHANNEL_S;
    conf->phy.enable_fixed_tx_rate = false;
    // 协议配置
    conf->ptl.protocol = EASY_WIFI_DEFAULT_PTL;
    return ESP_OK;
}

/**
 * @brief 从NVS中获取配置,如果配置不存在则会写入默认配置
 * 
 * @param conf 
 * @param isOverride 如果此前不存在配置，或是配置的版本和当前不符，是否覆盖写入默认配置
 * @return esp_err_t 
 */
esp_err_t ewifi_basic_get_conf_from_nvs(ewifi_conf_t *conf,bool isOverride)
{
    // 开始
    ESP_LOGW(TAG,"开始从NVS中读取wifi配置,是否允许重写:%hhd",isOverride);
    esp_err_t err = ESP_OK;
    // 选中数据库
    envs_use(EASY_WIFI_NVS_NAMESPACE);
    // 获取配置
    err = envs_select(EASY_WIFI_NVS_KEY_NAME,(void*)conf, sizeof(ewifi_conf_t));
    // 如果配置不存在或长度不对
    if (err == ESP_ERR_INVALID_SIZE || err == ESP_ERR_NVS_NOT_FOUND) {
        if (isOverride) {
            ESP_LOGW(TAG,"NVS中的配置长度不对或不存在,将删除配置并重新写入默认配置");
            ewifi_basic_get_conf_from_default_ap(conf);
            envs_delete(EASY_WIFI_NVS_KEY_NAME);
            err = envs_insert(EASY_WIFI_NVS_KEY_NAME,(void*)conf, sizeof(ewifi_conf_t));
            if (err != ESP_OK) {
                ESP_LOGE(TAG,"向NVS中写入默认配置失败");
                return err;
            }
            ESP_LOGI(TAG,"从NVS中重写配置成功");
            return ESP_OK;
        }
        ESP_LOGE(TAG,"NVS中的配置长度不对或不存在,获取失败");
        return ESP_FAIL;
    }
    // 其他意外情况
    else if (err != ESP_OK) {
        ESP_LOGE(TAG,"从NVS中获取配置失败:%s",esp_err_to_name(err));
        return err;
    }
    // 成功结束
    ESP_LOGI(TAG,"从NVS中获取配置成功");
    return ESP_OK;
}

/**
 * @brief 将配置写入NVS
 * 
 * @param conf 
 * @param isOverride 如果此前不存在配置，或是配置的版本和当前不符，是否覆盖写入默认配置
 * @return esp_err_t 
 */
esp_err_t ewifi_basic_update_conf_to_nvs(ewifi_conf_t *conf,bool isOverride)
{
    // 开始
    ESP_LOGW(TAG,"开始向NVS中写入wifi配置,是否允许重写:%hhd",isOverride);
    esp_err_t err = ESP_OK;
    // 选中数据库
    envs_use(EASY_WIFI_NVS_NAMESPACE);
    // 写入配置
    err = envs_update(EASY_WIFI_NVS_KEY_NAME,(void*)conf, sizeof(ewifi_conf_t));
    // 如果配置不存在或长度不对
    if (err == ESP_ERR_INVALID_SIZE || err == ESP_ERR_NVS_NOT_FOUND) {
        if (isOverride) {
            ESP_LOGW(TAG,"NVS中的配置长度不对或不存在,将删除老版本并写入新版本");
            envs_delete(EASY_WIFI_NVS_KEY_NAME);
            err = envs_insert(EASY_WIFI_NVS_KEY_NAME,(void*)conf, sizeof(ewifi_conf_t));
            if (err != ESP_OK) {
                ESP_LOGE(TAG,"向NVS中写入配置失败");
                return err;
            }
            ESP_LOGI(TAG,"从NVS中重写配置成功");
            return ESP_OK;
        }
        ESP_LOGE(TAG,"NVS中的配置长度不对或不存在,获取失败");
        return ESP_FAIL;
    }
    // 其他意外情况
    else if (err != ESP_OK) {
        ESP_LOGE(TAG,"向NVS写入配置失败:%s",esp_err_to_name(err));
        return err;
    }
    // 成功结束
    ESP_LOGI(TAG,"向NVS写入配置成功");
    return ESP_OK;
}

/**
 * @brief 修改配置中的wifi名称和密码(一对函数)
 * 
 * @param conf 
 * @param ssid 
 * @param password 
 */
void ewifi_basic_set_ap_peri_ssid_password(ewifi_conf_t *conf,const char *ssid,const char *password)
{
    strcpy((char*)(conf->peri.config.ap.ssid), ssid);
    conf->peri.config.ap.ssid_len = strlen(ssid); //特别容易忘
    strcpy((char*)(conf->peri.config.ap.password), password);
}
void ewifi_basic_set_sta_peri_ssid_password(ewifi_conf_t *conf,const char *ssid,const char *password)
{
    strcpy((char*)(conf->peri.config.sta.ssid), ssid);
    strcpy((char*)(conf->peri.config.sta.password), password);
}

/**
 * @brief 修改配置中的wifi为802.11LR模式(一对函数)
 * 
 * @param conf 
 */
void ewifi_basic_set_ap_ptl_lr(ewifi_conf_t *conf)
{
    conf->ptl.protocol = WIFI_PROTOCOL_LR;
}
void ewifi_basic_set_sta_ptl_lr(ewifi_conf_t *conf)
{
    conf->ptl.protocol = WIFI_PROTOCOL_LR;
}

/**
 * @brief 将wifi的发送配置成固定速率(一对函数)
 * 
 * @param conf 
 * @param rate 
 * @warning wifi会自适应地调整TX速率，但一旦指定了，就会以指定的速率发送数据
 */
void ewifi_basic_set_ap_phy_fix_rate(ewifi_conf_t *conf,const wifi_tx_rate_config_t *rate)
{
    conf->phy.enable_fixed_tx_rate = true;
    memcpy(&(conf->phy.fixed_tx_rate),rate,sizeof(wifi_tx_rate_config_t));
}
void ewifi_basic_set_sta_phy_fix_rate(ewifi_conf_t *conf,const wifi_tx_rate_config_t *rate)
{
    conf->phy.enable_fixed_tx_rate = true;
    memcpy(&conf->phy.fixed_tx_rate,rate,sizeof(wifi_tx_rate_config_t));
}

esp_err_t ewifi_basic_init(ewifi_conf_t *conf)
{
    ESP_LOGW(TAG,"开始初始化wifi");
    // step1: 前提准备 ---------------------------------------------------------------
    ESP_LOGW(TAG,"wifi前提准备$ 初始化协议栈");
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_LOGW(TAG,"wifi前提准备$ 初始化Flash驱动");
    ESP_ERROR_CHECK(envs_init());
    ESP_LOGW(TAG,"wifi前提准备$ 初始化wifi回调函数");
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
    // step2: 外设配置 ---------------------------------------------------------------
    ESP_LOGW(TAG,"wifi外设初始化$ 前提准备");
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(conf->peri.mode));
    ESP_LOGW(TAG,"wifi外设初始化$ 地区配置");
    wifi_country_t cn_country = {
        .cc = "CN",
        .schan = 1,
        .nchan = 13,
        .policy = WIFI_COUNTRY_POLICY_MANUAL
    };
    ESP_ERROR_CHECK(esp_wifi_set_country(&cn_country));
    ESP_LOGW(TAG,"wifi外设初始化$ 配置wifi模式");
    if (conf->peri.mode == WIFI_MODE_AP){
        esp_netif_ap = wifi_init_softap(&conf->peri.config);
    }
    else if (conf->peri.mode == WIFI_MODE_STA){
        esp_netif_sta = wifi_init_sta(&conf->peri.config);
    }
    else if (conf->peri.mode == WIFI_MODE_APSTA){
        ESP_LOGE(TAG,"wifi-APSTA的代码尚未调通");
        return ESP_ERR_INVALID_ARG;
    }
    else{
        ESP_LOGE(TAG,"不允许的WIFI模式,请在AP/STA中选择其中1~2个");
        return ESP_ERR_INVALID_ARG;
    }
    ESP_LOGW(TAG,"wifi外设初始化$ 启动wifi");
    ESP_ERROR_CHECK(esp_wifi_start());
    // step3: 频段配置 ---------------------------------------------------------------
    ESP_LOGW(TAG,"wifi频段初始化$ 配置wifi频段");

    if (conf->ptl.protocol != WIFI_PROTOCOL_LR) {
        if (conf->peri.mode == WIFI_MODE_AP){
            ESP_ERROR_CHECK(esp_wifi_set_channel(conf->phy.primary_channel, conf->phy.secondary_channel));
            ESP_ERROR_CHECK(esp_wifi_set_bandwidth(WIFI_IF_AP, conf->phy.bandwidth));
        }
        else if (conf->peri.mode == WIFI_MODE_STA){
            ESP_LOGI(TAG,"STA模式不使用频段配置,频段完全取决于AP");
        }
    }
    else {
        ESP_LOGI(TAG,"802.11LR协议,无论AP/STA,均不使用频段配置");
    }
    ESP_LOGW(TAG,"wifi频段初始化$ 配置wifi速率");
    if (conf->phy.enable_fixed_tx_rate){
        wifi_interface_t ifx = conf->peri.mode == WIFI_MODE_AP ? WIFI_IF_AP : WIFI_IF_STA;
        esp_wifi_config_80211_tx(ifx, &(conf->phy.fixed_tx_rate));
    }
    else{
        ESP_LOGI(TAG,"不使用固定速率,由wifi自适应调整");
    }
    // step4: 协议配置 ---------------------------------------------------------------
    ESP_LOGW(TAG,"wifi协议初始化$ 配置wifi协议");
    if (conf->peri.mode == WIFI_MODE_AP){
        esp_wifi_set_protocol(WIFI_IF_AP, conf->ptl.protocol);
    }
    else if (conf->peri.mode == WIFI_MODE_STA){
        esp_wifi_set_protocol(WIFI_IF_STA, conf->ptl.protocol);
    }
    ESP_LOGW(TAG,"wifi初始化成功");
    return ESP_OK;
}

/**
 * @brief 等待wifi联网成功
 * 
 * @param mode 
 * @return esp_err_t 
 */
esp_err_t ewifi_wait_connection(wifi_mode_t mode)
{
    ESP_LOGW(TAG,"wifi开始联网,联网模式为%d",mode);

    // STA模式联网
    if (mode == WIFI_MODE_STA) {
        ESP_LOGI(TAG,"等待STA外设准备完毕");
        xEventGroupWaitBits(s_wifi_event_group,WIFI_STA_READY_BIT,pdFALSE,pdFALSE,portMAX_DELAY);

        ESP_LOGI(TAG,"等待STA接入热点");
        esp_wifi_connect();
        EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);
        if (bits & WIFI_CONNECTED_BIT) {
            ESP_LOGI(TAG,"STA联网成功,可以开始使用链路层及以上应用");
            return ESP_OK;
        }
        if (bits & WIFI_FAIL_BIT) {
            ESP_LOGE(TAG,"无法连接指定热点，超过最大尝试次数");
            return ESP_FAIL;
        }
    }

    // AP模式联网
    else if (mode == WIFI_MODE_AP) {
        ESP_LOGI(TAG,"等待1个STA设备接入本热点");
        xEventGroupWaitBits(s_wifi_event_group,WIFI_AP_CONNECTED_BIT,pdTRUE,pdFALSE,portMAX_DELAY);
        vTaskDelay(50); //暂时用死等的方式等待分配IP地址
        ESP_LOGI(TAG,"AP联网成功,可以开始使用链路层及以上应用");
        return ESP_OK;
    }
    
    // APSTA模式联网
    else if (mode == WIFI_MODE_APSTA){
        ESP_LOGI(TAG,"等待1个STA设备接入本热点");
        xEventGroupWaitBits(s_wifi_event_group,WIFI_AP_CONNECTED_BIT,pdTRUE,pdFALSE,portMAX_DELAY);

        ESP_LOGI(TAG,"等待STA外设准备完毕");
        xEventGroupWaitBits(s_wifi_event_group,WIFI_STA_READY_BIT,pdFALSE,pdFALSE,portMAX_DELAY);

        ESP_LOGI(TAG,"等待STA接入热点");
        EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                               WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                               pdFALSE,
                                               pdFALSE,
                                               portMAX_DELAY);
        if (bits & WIFI_CONNECTED_BIT) {
            ESP_LOGI(TAG, "STA联网成功,开始配置AP-STA的DNS服务器");
            softap_set_dns_addr(esp_netif_ap,esp_netif_sta);
        } 
        else if (bits & WIFI_FAIL_BIT) {
            ESP_LOGE(TAG,"无法连接指定热点，超过最大尝试次数");
            return ESP_FAIL;
        } 
        else {
            ESP_LOGE(TAG, "未知事件");
        }

        ESP_LOGI(TAG,"开始配置AP-STA的NAPT数据转发");
        esp_netif_set_default_netif(esp_netif_sta);
        if (esp_netif_napt_enable(esp_netif_ap) != ESP_OK) {
            ESP_LOGE(TAG, "NAPT在适配器上未开启: %p", esp_netif_ap);
        }

        ESP_LOGI(TAG,"APSTA联网成功,可以开始使用链路层及以上应用");
        return ESP_OK;
    }
    return ESP_ERR_NOT_ALLOWED;
}

/**
 * @brief wifi重连
 * 
 * @param mode 
 * @return esp_err_t 
 * @note ESP32的wifi有容错设计，因此无法立刻得知断连的发生，需要由外界轮询判断，可以调用本函数重连
 */
esp_err_t ewifi_basic_re_connect(wifi_mode_t mode)
{
    return esp_wifi_connect();
}

/* 私有函数定义 ---------------------------------------------------------------------*/

static void wifi_event_handler(void *arg, esp_event_base_t event_base,int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *) event_data;
        ESP_LOGI(TAG, "设备 "MACSTR" 接入WIFI, AID=%d",MAC2STR(event->mac), event->aid);
        xEventGroupSetBits(s_wifi_event_group, WIFI_AP_CONNECTED_BIT);
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *) event_data;
        ESP_LOGI(TAG, "设备 "MACSTR" 断开WIFI, AID=%d, 原因:%d",MAC2STR(event->mac), event->aid, event->reason);
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "STA初始化完成,等待接入热点");
        xEventGroupSetBits(s_wifi_event_group, WIFI_STA_READY_BIT);
    } 
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
        ESP_LOGI(TAG, "取得IP:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_AP_STAIPASSIGNED) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
        ESP_LOGI(TAG,"成功为STA赋予一个IP:" IPSTR,IP2STR(&event->ip_info.ip));
        // 未知原因，此事件不可用
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        s_retry_num ++;
        ESP_LOGE(TAG,"断开与AP的连接(后知后觉),重试次数:%d", s_retry_num);
        esp_wifi_connect();
    }
}

static esp_netif_t *wifi_init_softap(const wifi_config_t *wifi_ap_config)
{
    esp_netif_t *esp_netif_ap = esp_netif_create_default_wifi_ap();
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, (wifi_config_t*)wifi_ap_config));
    ESP_LOGI(TAG, "WIFI热点初始化完成. 名称:%s 密码:%s 信道:%d",wifi_ap_config->ap.ssid, wifi_ap_config->ap.password, wifi_ap_config->ap.channel);
    return esp_netif_ap;
}
static esp_netif_t *wifi_init_sta(const wifi_config_t *wifi_sta_config)
{
    esp_netif_t *esp_netif_sta = esp_netif_create_default_wifi_sta();
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, (wifi_config_t*)wifi_sta_config));
    ESP_LOGI(TAG, "WIFI接入点初始化完成. 名称:%s 密码:%s 信道全扫描",wifi_sta_config->sta.ssid, wifi_sta_config->sta.password);
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