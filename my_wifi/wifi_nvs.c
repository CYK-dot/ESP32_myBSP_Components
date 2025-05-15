#include <nvs_flash.h>
#include <esp_wifi.h>
#include <esp_err.h>
#include <esp_log.h>

#include "my_wifi.h"
static const char *TAG = "MyWifi-NVS";
static MyNvsWifiConf_t *g_defaultWifiConf;

#define CONFIG_MyWIFI_NVS_NAMESPACE "MyWifi"
#define CONFIG_MyWIFI_NVS_KEY "WifiConfig"

/**
 * @brief 初始化wifi配置在NVS分区中的位置
 * 
 * @param conf 默认的配置，必须给定
 * @return esp_err_t 
 * 
 * @note 默认配置仅会在第一次读取配置时写入flash，其他时候完全不会使用。因此建议填入const MyNvsWifiConf_t，节省RAM空间。
 */
esp_err_t MyWifiNvsInit(const MyNvsWifiConf_t *conf)
{
    ESP_LOGW(TAG,"开始初始化NVS分区");
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );
    g_defaultWifiConf = (MyNvsWifiConf_t*)conf;
    ESP_LOGI(TAG,"初始化NVS分区成功");
    return ESP_OK;
}

/**
 * @brief 从NVS中读取wifi配置
 * 
 * @param conf 
 * @return esp_err_t 
 */
esp_err_t MyWifiNvsRead(MyNvsWifiConf_t *conf)
{
    ESP_LOGW(TAG,"开始读取");
    nvs_handle_t my_handle;
    esp_err_t err;
    size_t required_size = 0;

    if (conf == NULL || g_defaultWifiConf == NULL){
        ESP_LOGE(TAG,"参数不能为NULL，或是尚未初始化");
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG,"开始寻找分区");
    err = nvs_open(CONFIG_MyWIFI_NVS_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG,"分区寻找失败!错误码%d",err);
        return err;
    }

    ESP_LOGI(TAG,"开始在分区内读取wifi配置");
    err = nvs_get_blob(my_handle, CONFIG_MyWIFI_NVS_KEY, NULL, &required_size);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE(TAG,"分区不存在！");
        return err;
    }
    if (required_size == 0) {
        ESP_LOGI(TAG,"wifi配置为空，给定初始值");
        MyWifiNvsWrite(g_defaultWifiConf);
    } 
    else if (required_size == sizeof(MyNvsWifiConf_t)){
        ESP_LOGI(TAG,"wifi配置非空,长度%d,准备读取配置",required_size);
        err = nvs_get_blob(my_handle, CONFIG_MyWIFI_NVS_KEY, conf, &required_size);
        if (err != ESP_OK) {
            ESP_LOGE(TAG,"未知错误！错误码%d",err);
            return err;
        }
    }
    else{
        ESP_LOGE(TAG,"NVS内的KEY大小不正确!");
        return ESP_ERR_INVALID_SIZE;
    }

    nvs_close(my_handle);
    return ESP_OK;
}

/**
 * @brief 将wifi配置写入NVS中
 * 
 * @param conf 
 * @return esp_err_t 
 */
esp_err_t MyWifiNvsWrite(const MyNvsWifiConf_t *conf)
{
    ESP_LOGW(TAG,"开始写入");
    nvs_handle_t my_handle;
    esp_err_t err;

    if (conf == NULL || g_defaultWifiConf == NULL){
        ESP_LOGE(TAG,"参数不能为NULL，或是尚未初始化");
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG,"开始寻找分区");
    err = nvs_open(CONFIG_MyWIFI_NVS_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG,"分区寻找失败!错误码%d",err);
        return err;
    }

    ESP_LOGI(TAG,"开始写入分区");
    err = nvs_set_blob(my_handle, CONFIG_MyWIFI_NVS_KEY, conf,sizeof(MyNvsWifiConf_t));
    if (err != ESP_OK) {
        ESP_LOGE(TAG,"写入时出现未知错误！错误码%d",err);
        nvs_close(my_handle);
        return err;
    }

    err = nvs_commit(my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG,"提交时出现未知错误！错误码%d",err);
        nvs_close(my_handle);
        return err;
    }

    nvs_close(my_handle);
    ESP_LOGI(TAG,"写入成功");
    return ESP_OK;
}