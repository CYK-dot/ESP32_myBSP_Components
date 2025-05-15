#include <esp_log.h>
#include <esp_err.h>
#include <esp_http_server.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <cJSON.h>
#include <sys/stat.h>

#include "my_wifi.h"

#define EXT_FILE(f)                                                 \
    extern const uint8_t f##_start[] asm("_binary_"#f"_start"); \
    extern const uint8_t f##_end[] asm("_binary_"#f"_end")
EXT_FILE(wifi_config_html);

#define WIFI_CONFIG_JSON_AP_SSID "ap_ssid"
#define WIFI_CONFIG_JSON_AP_PSWD "ap_password"
#define WIFI_CONFIG_JSON_AP_CHANNEL "channel"
#define WIFI_CONFIG_JSON_STA_SSID "sta_ssid"
#define WIFI_CONFIG_JSON_STA_PSWD "sta_password"
#define WIFI_CONFIG_JSON_MODE "mode"

#define WIFI_CONFIG_DEFAULT_AP_SSID CONFIG_DEFAULT_AP_SSID
#define WIFI_CONFIG_DEFAULT_AP_PSWD CONFIG_DEFAULT_AP_PSWD
#define WIFI_CONFIG_DEFAULT_AP_CHANNEL CONFIG_DEFAULT_AP_CHANNEL
#define WIFI_CONFIG_DEFAULT_STA_SSID CONFIG_DEFAULT_STA_SSID
#define WIFI_CONFIG_DEFAULT_STA_PSWD CONFIG_DEFAULT_STA_PSWD
#define WIFI_CONFIG_DEFAULT_MODE  WIFI_MODE_AP

static const char *TAG = "RscWifi-Http";

static httpd_handle_t *g_httpdWifi = NULL;
static MyNvsWifiConf_t g_wifiDefault = {.mode = WIFI_CONFIG_DEFAULT_MODE};

static inline esp_err_t prvSetupWifi(httpd_handle_t *httpd,const MyHttpWifiConf_t *conf);
static inline esp_err_t prvSetupHttp(httpd_handle_t *httpd,const MyHttpWifiConf_t *conf);

static esp_err_t handler_get_config(httpd_req_t *req);
static esp_err_t handler_post_config(httpd_req_t* req);
static esp_err_t handler_post_reboot(httpd_req_t* req);

/**
 * @brief 通过http初始化wifi
 * 
 * @param httpd 
 * @param conf 
 * @return esp_err_t 
 */
esp_err_t MyWifiHttpSetup(httpd_handle_t *httpd,const MyHttpWifiConf_t *conf)
{
    esp_err_t err = ESP_OK;

    if (httpd == NULL || conf == NULL ){
        ESP_LOGE(TAG,"参数不对，不应该是NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    //初始化参数
    err |= MyWifiSetConfigDefault(
        WIFI_CONFIG_DEFAULT_AP_SSID,
        WIFI_CONFIG_DEFAULT_AP_PSWD,
        strlen(WIFI_CONFIG_DEFAULT_AP_SSID),
        strlen(WIFI_CONFIG_DEFAULT_AP_PSWD),
        WIFI_MODE_AP,
        &g_wifiDefault.apConf);
    err |= MyWifiSetConfigDefault(
        WIFI_CONFIG_DEFAULT_STA_SSID,
        WIFI_CONFIG_DEFAULT_STA_PSWD,
        strlen(WIFI_CONFIG_DEFAULT_STA_SSID),
        strlen(WIFI_CONFIG_DEFAULT_STA_PSWD),
        WIFI_MODE_STA,
        &g_wifiDefault.staConf);
    err |= MyWifiSetConfigChannel(WIFI_CONFIG_DEFAULT_AP_CHANNEL,&g_wifiDefault.apConf);

    //初始化外设
    err |= MyWifiNvsInit(&g_wifiDefault);
    err |= prvSetupWifi(httpd,conf);
    err |= prvSetupHttp(httpd,conf);

    return err;
}

/**
 * @brief 初始化wifi外设并等待wifi完成配置
 * 
 * @return esp_err_t 
 */
static inline esp_err_t prvSetupWifi(httpd_handle_t *httpd,const MyHttpWifiConf_t *conf)
{
    esp_err_t err;
    //从NVS中读取配置
    err = MyWifiNvsRead(&g_wifiDefault);

    //初始化wifi
    err = MyWifiSetup(&g_wifiDefault.apConf,&g_wifiDefault.staConf,g_wifiDefault.mode);
    return err;
}

/**
 * @brief 初始化http协议栈,并为http注册wifi配置相关的uri
 * 
 * @param httpd 
 * @param html_path_in_spiffs 
 * @return esp_err_t 
 */
static inline esp_err_t prvSetupHttp(httpd_handle_t *httpd,const MyHttpWifiConf_t *conf)
{
    esp_err_t err = ESP_OK;

    //获取服务器句柄
    g_httpdWifi = httpd;

    //获取http配置
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    //为http注册uri
    httpd_start(g_httpdWifi, &config);

    httpd_uri_t welcome = {
            .uri       = conf->uriGetPage,           
            .method    = HTTP_GET,           
            .handler   = handler_get_config,  
            .user_ctx  = NULL            
    };
    err |= httpd_register_uri_handler(*g_httpdWifi, &welcome);

    httpd_uri_t submit = {
            .uri       = conf->uriPostConfig,           
            .method    = HTTP_POST,            
            .handler   = handler_post_config,  
            .user_ctx  = NULL     
    };
    err |= httpd_register_uri_handler(*g_httpdWifi, &submit);

    httpd_uri_t reboot ={
            .uri       = conf->uriPostReboot,
            .method    = HTTP_POST,
            .handler   = handler_post_reboot,
            .user_ctx  = NULL
    };
    err |= httpd_register_uri_handler(*g_httpdWifi, &reboot);
    return err;
}

/**
 * @brief http客户端发起/config请求,ESP32后端自动调用该回调函数把html文件传回
 * 
 * @param req 
 * @return esp_err_t 
 */
static esp_err_t handler_get_config(httpd_req_t *req)
{
    ESP_LOGI(TAG,"客户端请求wifi配置界面");
    //返回数据给客户端
    esp_err_t response;
    response = httpd_resp_send(req, (char*)wifi_config_html_start, HTTPD_RESP_USE_STRLEN);
    return response;
}

/**
 * @brief http客户端发起/submit请求,ESP32后端自动调用该回调函数获取wifi配置
 * 
 * @param req 
 * @return esp_err_t 
 */
static esp_err_t handler_post_config(httpd_req_t* req)
{
    char* rcv_buffer;
    esp_err_t err;
    
    if (req->content_len > 0){
        //开辟接收空间
        rcv_buffer = malloc(req->content_len);
        if (rcv_buffer == NULL){
            httpd_resp_send_500(req);
            ESP_LOGE(TAG,"wifi-html-config堆空间耗尽");
            return ESP_ERR_NO_MEM;
        }
        //接收数据
        int rcv_len = httpd_req_recv(req,rcv_buffer,req->content_len);
        rcv_buffer[rcv_len] = '\0';
        ESP_LOGW(TAG,"客户端发来的数据：");
        ESP_LOGI(TAG,"%s",rcv_buffer);
        //解析数据
        cJSON *json = cJSON_Parse(rcv_buffer);
        cJSON *json_ssid = cJSON_GetObjectItem(json,WIFI_CONFIG_JSON_AP_SSID);
        cJSON *json_pswd = cJSON_GetObjectItem(json,WIFI_CONFIG_JSON_AP_PSWD);
        cJSON *json_channel = cJSON_GetObjectItem(json,WIFI_CONFIG_JSON_AP_CHANNEL);
        cJSON *json_sta_ssid = cJSON_GetObjectItem(json,WIFI_CONFIG_JSON_STA_SSID);
        cJSON *json_sta_pswd = cJSON_GetObjectItem(json,WIFI_CONFIG_JSON_STA_PSWD);
        cJSON *json_mode = cJSON_GetObjectItem(json,WIFI_CONFIG_JSON_MODE);
        if (json == NULL || json_ssid == NULL || json_pswd == NULL || json_channel == NULL){
            ESP_LOGE(TAG,"JSON解析失败！");
            free(rcv_buffer);
            cJSON_Delete(json_ssid);
            cJSON_Delete(json_pswd);
            cJSON_Delete(json_channel);
            cJSON_Delete(json_sta_ssid);
            cJSON_Delete(json_sta_pswd);
            cJSON_Delete(json_mode);
            cJSON_Delete(json);
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }
        ESP_LOGW(TAG,"数据解析完毕！");

        //将配置写入配置结构体
        err |= MyWifiSetConfigDefault(
            json_ssid->valuestring,
            json_pswd->valuestring,
            strlen(json_ssid->valuestring),
            strlen(json_pswd->valuestring),
            WIFI_MODE_AP,
            &g_wifiDefault.apConf);

        err |= MyWifiSetConfigDefault(
            json_sta_ssid->valuestring,
            json_sta_pswd->valuestring,
            strlen(json_sta_ssid->valuestring),
            strlen(json_sta_pswd->valuestring),
            WIFI_MODE_STA,
            &g_wifiDefault.staConf);
            
        err |= MyWifiSetConfigChannel(json_channel->valueint,&g_wifiDefault.apConf);

        //将配置写入NVS
        err |= MyWifiNvsWrite(&g_wifiDefault);
        //给客户端反馈
        const char resp[] = "配置已写入NVS,重启后生效。";
        httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);

        free(rcv_buffer);
        cJSON_Delete(json);
        // cJSON_Delete(json_ssid);
        // cJSON_Delete(json_pswd);
        // cJSON_Delete(json_channel);
        // cJSON_Delete(json_sta_ssid);
        // cJSON_Delete(json_sta_pswd);
        // cJSON_Delete(json_mode);
    }
    return ESP_OK;
}

/**
 * @brief http客户端发起/reboot请求,ESP32后端自动调用该回调函数重启
 * 
 * @param req 
 * @return esp_err_t 
 */
static esp_err_t handler_post_reboot(httpd_req_t* req)
{
    //给客户端反馈
    const char resp[] = "ESP32将会在1秒后重启。";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    vTaskDelay(100);//等待数据发送到http客户端之后再重启
    esp_restart();
    return ESP_OK;
}