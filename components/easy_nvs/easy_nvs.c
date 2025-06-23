/**
 * @file easy_nvs.c
 * @author CYK-Dot
 * @brief 将NVS操作封装成更易用的接口
 * @version 0.1
 * @date 2025-06-23
 *
 * @copyright Copyright (c) 2025
 */

/* 头文件引入 -----------------------------------------------------------------------*/
#include "easy_nvs.h"

#include <string.h>
#include <stdio.h>
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include "esp_err.h"

/* 私有类型定义 ---------------------------------------------------------------------*/

/* 私有宏定义 -----------------------------------------------------------------------*/
static const char *TAG = "EASY_NVS";

/* 全局变量声明 ---------------------------------------------------------------------*/
static char current_namespace[16] = {0};
static bool nvs_initialized = false;

/* 私有函数声明 ---------------------------------------------------------------------*/
static esp_err_t envs_validate_params(const char *key, const void *ptr, size_t size, bool check_namespace);

/* 导出函数定义 ---------------------------------------------------------------------*/

/**
 * @brief 初始化NVS（如果尚未初始化）
 * @return ESP_OK 成功，其他值表示失败
 */
esp_err_t envs_init(void)
{
    if (nvs_initialized) {
        return ESP_OK;
    }
    
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS分区需要擦除，正在重新初始化");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS初始化失败: %s", esp_err_to_name(ret));
        return ret;
    }
    
    nvs_initialized = true;
    ESP_LOGI(TAG, "NVS初始化成功");
    return ESP_OK;
}

/**
 * @brief 设置当前使用的命名空间
 * @param namespace 命名空间名称
 * @return ESP_OK 成功，其他值表示失败
 */
esp_err_t envs_use(const char *namespace)
{
    esp_err_t ret;
    if (!namespace) {
        ESP_LOGE(TAG, "命名空间不能为空");
        return ESP_ERR_INVALID_ARG;
    }
    
    if (strlen(namespace) == 0) {
        ESP_LOGE(TAG, "命名空间不能为空字符串");
        return ESP_ERR_INVALID_ARG;
    }
    
    if (strlen(namespace) > 15) {
        ESP_LOGE(TAG, "命名空间长度不能超过15个字符，当前长度: %d", strlen(namespace));
        return ESP_ERR_INVALID_ARG;
    }
    
    // 测试命名空间是否可用
    nvs_handle_t handle;
    ret = nvs_open(namespace, NVS_READONLY, &handle);
    if (ret != ESP_OK && ret != ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE(TAG, "无法打开命名空间 '%s': %s", namespace, esp_err_to_name(ret));
        return ret;
    }
    
    if (ret == ESP_OK) {
        nvs_close(handle);
    }
    
    // 保存命名空间
    strncpy(current_namespace, namespace, sizeof(current_namespace) - 1);
    current_namespace[sizeof(current_namespace) - 1] = '\0';
    
    ESP_LOGI(TAG, "设置命名空间为: %s", current_namespace);
    return ESP_OK;
}

/**
 * @brief 从NVS中读取数据
 * @param key 键名
 * @param out_value 输出缓冲区
 * @param size 缓冲区大小
 * @return ESP_OK 成功，其他值表示失败
 */
esp_err_t envs_select(const char *key, void *out_value, size_t size)
{
    esp_err_t ret = envs_validate_params(key, out_value, size, true);
    if (ret != ESP_OK) {
        return ret;
    }
    
    if (!out_value) {
        ESP_LOGE(TAG, "输出缓冲区不能为空");
        return ESP_ERR_INVALID_ARG;
    }
    
    nvs_handle_t handle;
    ret = nvs_open(current_namespace, NVS_READONLY, &handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "无法打开命名空间 '%s' 进行读取: %s", current_namespace, esp_err_to_name(ret));
        return ret;
    }
    
    size_t required_size = size;
    ret = nvs_get_blob(handle, key, out_value, &required_size);
    
    if (ret == ESP_OK) {
        if (required_size > size) {
            ESP_LOGW(TAG, "缓冲区大小不足，需要 %d 字节，提供了 %d 字节,请考虑是否为版本兼容问题", required_size, size);
            nvs_close(handle);
            return ESP_ERR_INVALID_SIZE;
        }
        ESP_LOGD(TAG, "成功读取键 '%s'，数据大小: %d 字节", key, required_size);
    } else if (ret == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "键 '%s' 不存在", key);
    } else {
        ESP_LOGE(TAG, "读取键 '%s' 失败: %s", key, esp_err_to_name(ret));
    }
    
    nvs_close(handle);
    return ret;
}

/**
 * @brief 更新NVS中已存在的数据
 * @param key 键名
 * @param value 要写入的数据
 * @param size 数据大小
 * @return ESP_OK 成功，其他值表示失败
 */
esp_err_t envs_update(const char *key, const void *value, size_t size)
{
    esp_err_t ret = envs_validate_params(key, value, size, true);
    if (ret != ESP_OK) {
        return ret;
    }
    
    if (!value) {
        ESP_LOGE(TAG, "要更新的数据不能为空");
        return ESP_ERR_INVALID_ARG;
    }
    
    // 首先检查键是否存在
    nvs_handle_t handle;
    ret = nvs_open(current_namespace, NVS_READONLY, &handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "无法打开命名空间 '%s' 进行检查: %s", current_namespace, esp_err_to_name(ret));
        return ret;
    }
    
    size_t required_size = 0;
    ret = nvs_get_blob(handle, key, NULL, &required_size);
    nvs_close(handle);
    
    if (ret == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE(TAG, "键 '%s' 不存在，无法更新。请使用envs_insert()创建新键", key);
        return ESP_ERR_NVS_NOT_FOUND;
    } else if (ret != ESP_OK) {
        ESP_LOGE(TAG, "检查键 '%s' 存在性时失败: %s", key, esp_err_to_name(ret));
        return ret;
    }
    
    // 打开命名空间进行写入
    ret = nvs_open(current_namespace, NVS_READWRITE, &handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "无法打开命名空间 '%s' 进行写入: %s", current_namespace, esp_err_to_name(ret));
        return ret;
    }
    
    ret = nvs_set_blob(handle, key, value, size);
    if (ret == ESP_OK) {
        ret = nvs_commit(handle);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "成功更新键 '%s'，数据大小: %d 字节", key, size);
        } else {
            ESP_LOGE(TAG, "提交更新键 '%s' 失败: %s", key, esp_err_to_name(ret));
        }
    } else {
        ESP_LOGE(TAG, "更新键 '%s' 失败: %s", key, esp_err_to_name(ret));
    }
    
    nvs_close(handle);
    return ret;
}

/**
 * @brief 向NVS中插入新数据
 * @param key 键名
 * @param value 要写入的数据
 * @param size 数据大小
 * @return ESP_OK 成功，其他值表示失败
 */
esp_err_t envs_insert(const char *key, const void *value, size_t size)
{
    esp_err_t ret = envs_validate_params(key, value, size, true);
    if (ret != ESP_OK) {
        return ret;
    }
    
    if (!value) {
        ESP_LOGE(TAG, "要插入的数据不能为空");
        return ESP_ERR_INVALID_ARG;
    }
    
    // 首先检查键是否已存在
    nvs_handle_t handle;
    ret = nvs_open(current_namespace, NVS_READONLY, &handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "无法打开命名空间 '%s' 进行检查: %s", current_namespace, esp_err_to_name(ret));
        return ret;
    }
    
    size_t required_size = 0;
    ret = nvs_get_blob(handle, key, NULL, &required_size);
    nvs_close(handle);
    
    if (ret == ESP_OK) {
        ESP_LOGE(TAG, "键 '%s' 已存在，无法插入。请使用envs_update()更新现有键", key);
        return ESP_ERR_NVS_INVALID_STATE;
    } else if (ret != ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE(TAG, "检查键 '%s' 存在性时失败: %s", key, esp_err_to_name(ret));
        return ret;
    }
    
    // 打开命名空间进行写入
    ret = nvs_open(current_namespace, NVS_READWRITE, &handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "无法打开命名空间 '%s' 进行写入: %s", current_namespace, esp_err_to_name(ret));
        return ret;
    }
    
    ret = nvs_set_blob(handle, key, value, size);
    if (ret == ESP_OK) {
        ret = nvs_commit(handle);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "成功插入键 '%s'，数据大小: %d 字节", key, size);
        } else {
            ESP_LOGE(TAG, "提交插入键 '%s' 失败: %s", key, esp_err_to_name(ret));
        }
    } else {
        ESP_LOGE(TAG, "插入键 '%s' 失败: %s", key, esp_err_to_name(ret));
    }
    
    nvs_close(handle);
    return ret;
}

/**
 * @brief 从NVS中删除数据
 * @param key 键名
 * @return ESP_OK 成功，其他值表示失败
 */
esp_err_t envs_delete(const char *key)
{
    esp_err_t ret = envs_validate_params(key, NULL, 0, true);
    if (ret != ESP_OK) {
        return ret;
    }
    
    nvs_handle_t handle;
    ret = nvs_open(current_namespace, NVS_READWRITE, &handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "无法打开命名空间 '%s' 进行删除: %s", current_namespace, esp_err_to_name(ret));
        return ret;
    }
    
    ret = nvs_erase_key(handle, key);
    if (ret == ESP_OK) {
        ret = nvs_commit(handle);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "成功删除键 '%s'", key);
        } else {
            ESP_LOGE(TAG, "提交删除键 '%s' 失败: %s", key, esp_err_to_name(ret));
        }
    } else if (ret == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "键 '%s' 不存在，无需删除", key);
    } else {
        ESP_LOGE(TAG, "删除键 '%s' 失败: %s", key, esp_err_to_name(ret));
    }
    
    nvs_close(handle);
    return ret;
}

/* 私有函数定义 ---------------------------------------------------------------------*/

/**
 * @brief 验证参数有效性
 * @param key 键名
 * @param ptr 指针参数（可为NULL）
 * @param size 大小参数（可为0）
 * @param check_namespace 是否检查namespace
 * @return ESP_OK 参数有效，其他值表示参数无效
 */
static esp_err_t envs_validate_params(const char *key, const void *ptr, size_t size, bool check_namespace)
{
    if (check_namespace && strlen(current_namespace) == 0) {
        ESP_LOGE(TAG, "尚未设置命名空间，请先调用envs_use()");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (!key) {
        ESP_LOGE(TAG, "键名不能为空");
        return ESP_ERR_INVALID_ARG;
    }
    
    if (strlen(key) == 0) {
        ESP_LOGE(TAG, "键名不能为空字符串");
        return ESP_ERR_INVALID_ARG;
    }
    
    if (strlen(key) > 15) {
        ESP_LOGE(TAG, "键名长度不能超过15个字符，当前长度: %d", strlen(key));
        return ESP_ERR_INVALID_ARG;
    }
    
    if (ptr && size == 0) {
        ESP_LOGE(TAG, "指针不为空时，大小不能为0");
        return ESP_ERR_INVALID_ARG;
    }
    
    if (!ptr && size > 0) {
        ESP_LOGE(TAG, "指针为空时，大小必须为0");
        return ESP_ERR_INVALID_ARG;
    }
    
    return ESP_OK;
}