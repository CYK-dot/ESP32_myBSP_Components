#pragma once

#ifdef __cplusplus
    extern "C" {
#endif

#include <esp_wifi.h>
#include <stdint.h>
#include <esp_http_server.h>

// 最为基本的wifi操作接口，不依赖于http，可单独使用
esp_err_t MyWifiSetConfigDefault(const char *ssid,const char *password,size_t ssid_len,size_t password_len,wifi_mode_t mode,wifi_config_t *config);
esp_err_t MyWifiSetConfigChannel(int channel,wifi_config_t *config);
esp_err_t MyWifiSetup(const wifi_config_t *apConfig,const wifi_config_t *staConfig,wifi_mode_t mode);
esp_err_t MyWifiWaitConnection(wifi_mode_t mode);
int MyWifiGetStaConnectCount(void);

// 最为基本的配置读取/写入接口，不依赖于http，可单独使用
typedef struct{
    wifi_config_t apConf;
    wifi_config_t staConf;
    wifi_mode_t mode;
}MyNvsWifiConf_t;
esp_err_t MyWifiNvsInit(const MyNvsWifiConf_t *conf);
esp_err_t MyWifiNvsRead(MyNvsWifiConf_t *conf);
esp_err_t MyWifiNvsWrite(const MyNvsWifiConf_t *conf);

// 通过http配置的接口
typedef struct{
    char *uriGetPage;
    char *uriPostConfig;
    char *uriPostReboot;
}MyHttpWifiConf_t;
esp_err_t MyWifiHttpSetup(httpd_handle_t *httpd,const MyHttpWifiConf_t *conf);

#define MyHttpWifiConfigConstDefault \
{                                     \
    .uriGetPage = "/",                \
    .uriPostConfig = "/wifi_submit",  \
    .uriPostReboot = "/wifi_reboot",  \
}             

// 配置wifi的协议
typedef enum{
    WIFI_PTL_80211_N,    ///< wifi4
    WIFI_PTL_80211_AX,   ///< wifi6的2.4G版本
    WIFI_PTL_80211_LR,   ///< 乐鑫长距离wifi
}MyProtocolWifi_t;
esp_err_t MyWifiSetProtocol(wifi_interface_t ifx,wifi_bandwidth_t bd,MyProtocolWifi_t ptl);

#ifdef __cplusplus
    }
#endif