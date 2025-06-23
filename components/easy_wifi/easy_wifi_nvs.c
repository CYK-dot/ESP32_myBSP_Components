/**
 * @file easy_wifi_nvs.c
 * @author CYK-Dot
 * @brief 命令行式WiFi管理
 * @version 0.1
 * @date 2025-06-23
 *
 * @copyright Copyright (c) 2025
 */

/* 头文件引入 -----------------------------------------------------------------------*/
#include "easy_wifi.h"

#include <nvs_flash.h>
#include <esp_wifi.h>
#include <esp_event.h>

/* 私有类型定义 ---------------------------------------------------------------------*/

/* 私有宏定义 -----------------------------------------------------------------------*/
static const char *TAG = "easy_wifi";

/* 全局变量声明 ---------------------------------------------------------------------*/

/* 私有函数声明 ---------------------------------------------------------------------*/

static inline void prv_debug_conf_show(const ewifi_conf_t *conf);
static inline esp_err_t prv_fetch_conf_from_default(ewifi_conf_t *conf);
static inline esp_err_t prv_fetch_conf_from_nvs(ewifi_conf_t *conf);
static inline esp_err_t prv_update_conf_to_nvs(const ewifi_conf_t *conf);

/* 导出函数定义 ---------------------------------------------------------------------*/

/**
 * @brief 初始化easy_wifi的NVS介质
 * 
 * @return esp_err_t 
 */
esp_err_t ewifi_nvs_init(void)
{
    ESP_LOGI(TAG, "初始化easy_wifi_nvs");
    // step1: 初始化NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGE(TAG, "NVS初始化失败,擦除并重新初始化");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }
    
    // step2: 从NVS中读取上一次的配置
    ewifi_conf_t conf;
    err = prv_fetch_conf_from_nvs(&conf);

    // step3: 如果NVS中不存在上一次的配置, 则从默认配置中读取
    if (err == ESP_ERR_NOT_FOUND) {
        ESP_LOGW(TAG, "未能从NVS中读取到配置, 使用默认配置");
        prv_fetch_conf_from_default(&conf);
        err = prv_update_conf_to_nvs(&conf);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "easy_wifi_nvs初始化失败,错误为(%s)", esp_err_to_name(err));
            return err;
        }
    }

    // step3: 如果NVS中存储的配置版本不对劲，那么就清除并重新写入默认配置
    if (err == ESP_ERR_INVALID_SIZE) {
        ESP_LOGW(TAG, "NVS中存储的wifi配置大小不正确, 存在版本变更，清除并重新写入默认配置");
        nvs_flash_erase();
        err = prv_fetch_conf_from_default(&conf);
        err = prv_update_conf_to_nvs(&conf);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "easy_wifi_nvs初始化失败,错误为(%s)", esp_err_to_name(err));
            return err;
        }
    }
    prv_debug_conf_show(&conf);
    ESP_LOGI(TAG, "easy_wifi_nvs初始化成功");
    return ESP_OK;
}

/**
 * @brief 清除NVS中存储的WiFi配置
 * 
 * @return esp_err_t 
 */
esp_err_t ewifi_nvs_reset(void)
{
    esp_err_t err;
    nvs_handle_t handle;

    // step1: 打开NVS命名空间（读写模式）
    err = nvs_open(EASY_WIFI_NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "无法打开NVS命名空间 (%s)", esp_err_to_name(err));
        return err;
    }

    // step2: 尝试删除键
    err = nvs_erase_key(handle, EASY_WIFI_NVS_KEY_NAME);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "配置项不存在，无需清除");
        nvs_close(handle);
        return ESP_OK;  // 配置不存在也算成功
    } 
    else if (err != ESP_OK) {
        ESP_LOGE(TAG, "清除配置项失败 (%s)", esp_err_to_name(err));
        nvs_close(handle);
        return err;
    }

    // step3: 提交更改
    err = nvs_commit(handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "提交删除操作失败 (%s)", esp_err_to_name(err));
    } 
    else {
        ESP_LOGI(TAG, "配置项清除成功");
    }
    nvs_close(handle);
    return err;
}


/* 私有函数定义 ---------------------------------------------------------------------*/

/**
 * @brief 展示配置
 * 
 * @param conf 
 */
static inline void prv_debug_conf_show(const ewifi_conf_t *conf)
{
    ESP_LOGI(TAG, "当前WiFi配置如下:");
    ESP_LOGI(TAG, "SSID: %s", conf->ssid);
    ESP_LOGI(TAG, "Password: %s", conf->password);
    ESP_LOGI(TAG, "Mode: %d", conf->mode);
}

/**
 * @brief 从宏定义中获取默认配置
 * 
 * @param conf 
 * @return esp_err_t 
 */
static inline esp_err_t prv_fetch_conf_from_default(ewifi_conf_t *conf)
{
    memcpy(conf->ssid, EASY_WIFI_DEFAULT_SSID, sizeof(EASY_WIFI_DEFAULT_SSID));
    memcpy(conf->password, EASY_WIFI_DEFAULT_PASSWORD, sizeof(EASY_WIFI_DEFAULT_PASSWORD));
    conf->mode = EASY_WIFI_DEFAULT_MODE;
    return ESP_OK;
}
/**
 * @brief 从NVS中读取配置
 * 
 * @param conf 需要读取的配置
 * @return esp_err_t 是否读取成功
 *         ESP_ERR_INVALID_SIZE -- 新版本更改了ewifi_conf_t,导致内存长度变化
 *         ESP_ERR_NOT_FOUND    -- NVS中不存在上一次的配置
 */
static inline esp_err_t prv_fetch_conf_from_nvs(ewifi_conf_t *conf)
{
    esp_err_t err;
    nvs_handle_t handle;
    ewifi_conf_t retval;
    size_t required_size = sizeof(ewifi_conf_t);
    // step1: 打开NVS命名空间
    err = nvs_open(EASY_WIFI_NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "NVS命名空间 (%s) 不存在", esp_err_to_name(err));
        return err;
    }
    // step2: 从NVS中读取上一次的配置
    err = nvs_get_blob(handle, EASY_WIFI_NVS_KEY_NAME, &retval, &required_size);
    if (err == ESP_OK && required_size == sizeof(ewifi_conf_t)) {
        ESP_LOGI(TAG, "从NVS中成功读取上一次的wifi配置");
        memcpy(conf, &retval, sizeof(ewifi_conf_t));
        nvs_close(handle);
        return ESP_OK;
    }
    else if (err == ESP_OK && required_size != sizeof(ewifi_conf_t)) {
        ESP_LOGW(TAG, "NVS中存储的wifi配置大小不正确, 存在版本变更，读取终止");
        nvs_close(handle);
        return ESP_ERR_INVALID_SIZE;
    }
    else if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "NVS中不存在上一次的wifi配置, 读取终止");
        nvs_close(handle);
        return ESP_ERR_NOT_FOUND;
    } 
    else {
        ESP_LOGE(TAG, "读取NVS失败,错误为(%s)", esp_err_to_name(err));
        nvs_close(handle);
        return err;
    }
}

/**
 * @brief 将配置写入NVS
 * 
 * @param conf 
 * @return esp_err_t 
 */
static inline esp_err_t prv_update_conf_to_nvs(const ewifi_conf_t *conf)
{
    esp_err_t err;
    nvs_handle_t handle;
    // step1: 打开NVS命名空间
    err = nvs_open(EASY_WIFI_NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "NVS命名空间 (%s) 不存在", esp_err_to_name(err));
        return err;
    }
    // step2: 将配置写入NVS
    err = nvs_set_blob(handle, EASY_WIFI_NVS_KEY_NAME, conf, sizeof(ewifi_conf_t));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "写入NVS失败,错误为(%s)", esp_err_to_name(err));
        nvs_close(handle);
        return err;
    }
    // step3: 提交写入
    err = nvs_commit(handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "提交写入失败,错误为(%s)", esp_err_to_name(err));
        nvs_close(handle);
        return err;
    }
    nvs_close(handle);
    return ESP_OK;
}