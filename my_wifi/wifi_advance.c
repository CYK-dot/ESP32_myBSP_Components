#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"

#include "my_wifi.h"

static const char *TAG_COMMON = "MyWifi-COM";

/**
 * @brief 配置wifi的带宽和协议
 * 
 * @param ifx 可以填入WIFI_IF_AP/WIFI_IF_STA
 * @param bd  802.11N下，可以填入WIFI_BW_HT20/WIFI_BW_HT40
 *            802.11AX下此参数无效
 *            802.11LR下此参数无效
 * @param ptl 
 * @return esp_err_t 
 */
esp_err_t MyWifiSetProtocol(wifi_interface_t ifx,wifi_bandwidth_t bd,MyProtocolWifi_t ptl)
{
    esp_err_t err;
   
    switch(ptl) {
        case WIFI_PTL_80211_AX:
            ESP_LOGW(TAG_COMMON,"配置接口%d的协议为802.11AX",ifx);
            err = esp_wifi_set_bandwidth(ifx,WIFI_BW_HT20);
            if (err != ESP_OK) {
                ESP_LOGE(TAG_COMMON,"配置带宽失败，错误码%#X",err);
                return err;
            }
            err = esp_wifi_set_protocol(ifx,WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N|WIFI_PROTOCOL_11AX);
            if (err != ESP_OK) {
                ESP_LOGE(TAG_COMMON,"配置协议失败，错误码%#X",err);
                return err;
            }
        break;

        case WIFI_PTL_80211_N:
            ESP_LOGW(TAG_COMMON,"配置接口%d的协议为802.11N",ifx);
            err = esp_wifi_set_bandwidth(ifx,bd);
            if (err != ESP_OK) {
                ESP_LOGE(TAG_COMMON,"配置带宽失败，错误码%#X",err);
                return err;
            }
            err = esp_wifi_set_protocol(ifx,WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N);
            if (err != ESP_OK) {
                ESP_LOGE(TAG_COMMON,"配置协议失败，错误码%d",err);
                return err;
            }
        break;

        case WIFI_PTL_80211_LR:
            ESP_LOGW(TAG_COMMON,"配置接口%d的协议为802.11LR",ifx);
            err = esp_wifi_set_protocol(ifx,WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N|WIFI_PROTOCOL_LR);
            if (err != ESP_OK) {
                ESP_LOGE(TAG_COMMON,"配置协议失败，错误码%#X",err);
                return err;
            }
        break;
    }
    return ESP_OK;
}