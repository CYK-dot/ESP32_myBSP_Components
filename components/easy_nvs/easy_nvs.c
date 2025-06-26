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

#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>

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
 * @brief 删除NVS内的所有数据
 * 
 * @return esp_err_t 
 */
esp_err_t envs_reset_all(void)
{
    ESP_LOGW(TAG, "NVS_RESET_ALL:删除NVS内的所有数据");
    esp_err_t ret = nvs_flash_erase();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS重置失败:%s", esp_err_to_name(ret));
        return ret;
    }
    nvs_initialized = false;
    ESP_LOGW(TAG, "NVS_RESET_ALL:删除成功");
    return ESP_OK;
}

/**
 * @brief 初始化NVS（如果尚未初始化）
 * @return ESP_OK 成功，其他值表示失败
 */
esp_err_t envs_init(void)
{
    ESP_LOGI(TAG, "NVS初始化开始");
    if (nvs_initialized) {
        return ESP_OK;
    }
    
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS分区不可用(%s)需要擦除,正在重新初始化",esp_err_to_name(ret));
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS初始化失败:%s", esp_err_to_name(ret));
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
esp_err_t envs_use(const char *prv_namespace)
{
    esp_err_t ret;
    if (!prv_namespace) {
        ESP_LOGE(TAG, "USE命名空间不能为空");
        return ESP_ERR_INVALID_ARG;
    }
    
    if (strlen(prv_namespace) == 0) {
        ESP_LOGE(TAG, "USE命名空间不能为空字符串");
        return ESP_ERR_INVALID_ARG;
    }
    
    if (strlen(prv_namespace) > 15) {
        ESP_LOGE(TAG, "USE命名空间长度不能超过15个字符，当前长度: %d", strlen(prv_namespace));
        return ESP_ERR_INVALID_ARG;
    }
    
    // 测试命名空间是否可用
    nvs_handle_t handle;
    ret = nvs_open(prv_namespace, NVS_READONLY, &handle);
    if (ret != ESP_OK && ret != ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE(TAG, "USE无法打开命名空间 '%s': %s", prv_namespace, esp_err_to_name(ret));
        return ret;
    }
    
    if (ret == ESP_OK) {
        nvs_close(handle);
    }
    
    // 保存命名空间
    strncpy(current_namespace, prv_namespace, sizeof(current_namespace) - 1);
    current_namespace[sizeof(current_namespace) - 1] = '\0';
    
    ESP_LOGI(TAG, "USE设置命名空间为: %s", current_namespace);
    return ESP_OK;
}

/**
 * @brief 从NVS中读取数据
 * @param key 键名
 * @param out_value 输出缓冲区
 * @param size 缓冲区大小
 * @return ESP_OK 成功
 *         ESP_ERR_INVALID_ARG 参数不对
 *         ESP_ERR_INVALID_SIZE 字段大小不匹配
 *         ESP_ERR_NVS_NOT_FOUND 字段不存在(严重)或是命名空间为空(可以忽视该错误)
 *         其他值表示未知失败,需要回来排查问题
 */
esp_err_t envs_select(const char *key, void *out_value, size_t size)
{
    ESP_LOGW(TAG, "SELECT %s FROM %s", key, current_namespace);
    esp_err_t ret = envs_validate_params(key, out_value, size, true);
    if (ret != ESP_OK) {
        return ret;
    }
    
    if (!out_value) {
        ESP_LOGE(TAG, "SELECT输出不能为空");
        return ESP_ERR_INVALID_ARG;
    }
    
    nvs_handle_t handle;
    ret = nvs_open(current_namespace, NVS_READONLY, &handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SELECT无法打开命名空间'%s':%s", current_namespace, esp_err_to_name(ret));
        return ret;
    }
    
    size_t required_size = size;
    ret = nvs_get_blob(handle, key, out_value, &required_size);
    if (ret == ESP_OK) {
        if (required_size != size) {
            ESP_LOGW(TAG, "SELECT字段大小不匹配,需要%dB,提供了%dB,请考虑是否为版本兼容问题", required_size, size);
            nvs_close(handle);
            return ESP_ERR_INVALID_SIZE;
        }
    } 
    else {
        ESP_LOGE(TAG, "SELECT读取字段'%s'失败:%s,若字段不存在,请使用INSERT增加该字段", key, esp_err_to_name(ret));
        nvs_close(handle);
        return ret;
    }
    
    nvs_close(handle);
    ESP_LOGW(TAG,"SELECT成功");
    return ret;
}

/**
 * @brief 更新NVS中已存在的数据
 * @param key 键名
 * @param value 要写入的数据
 * @param size 数据大小
 * @return ESP_OK 成功
 *         ESP_ERR_INVALID_ARG 参数不对
 *         ESP_ERR_INVALID_SIZE 字段大小不匹配
 *         ESP_ERR_NVS_NOT_FOUND 字段不存在
 *         其他值表示未知失败,需要回来排查问题
 */
esp_err_t envs_update(const char *key, const void *value, size_t size)
{
    ESP_LOGW(TAG, "UPDATE %s SET %s", current_namespace, key);
    esp_err_t ret = envs_validate_params(key, value, size, true);
    if (ret != ESP_OK) {
        return ret;
    }
    
    nvs_handle_t handle = 0;
    size_t required_size = 0;
    
    // 打开命名空间
    ret = nvs_open(current_namespace, NVS_READWRITE, &handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "UPDATE无法打开命名空间'%s':%s", current_namespace, esp_err_to_name(ret));
        return ret;
    }
    
    // 检查字段是否存在
    ret = nvs_get_blob(handle, key, NULL, &required_size);
    if (ret == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE(TAG, "UPDATE字段'%s'不存在.请使用INSERT创建新键", key);
        nvs_close(handle); 
        return ESP_ERR_NVS_NOT_FOUND;
    } 
    else if (required_size != size) {
        ESP_LOGW(TAG, "UPDATE字段'%s'大小不匹配,需要%dB,提供了%dB", key, required_size, size);
        nvs_close(handle); 
        return ESP_ERR_INVALID_SIZE;
    }
    else if (ret != ESP_OK) {
        ESP_LOGE(TAG, "UPDATE无法检查字段'%s'是否存在:%s", key, esp_err_to_name(ret));
        nvs_close(handle); // 关闭句柄
        return ret;
    }
    
    // 更新数据
    ret = nvs_set_blob(handle, key, value, size);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "UPDATE更新字段'%s'失败:%s", key, esp_err_to_name(ret));
        nvs_close(handle);
        return ret;
    }
    
    // 提交更改
    ret = nvs_commit(handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "UPDATE提交字段'%s'失败:%s", key, esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "UPDATE提交字段'%s'成功,数据大小%dB", key, size);
    }
    
    nvs_close(handle);
    
    if (ret == ESP_OK) {
        ESP_LOGW(TAG, "UPDATE成功"); // 仅成功时打印
    }
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
    ESP_LOGW(TAG,"INSERT INTO %s VALUES (%s)", current_namespace, key);
    esp_err_t ret = envs_validate_params(key, value, size, true);
    if (ret != ESP_OK) {
        return ret;
    }
    if (!value) {
        ESP_LOGE(TAG, "INSERT要插入的数据不能为空");
        return ESP_ERR_INVALID_ARG;
    }
    
    // step1:检查键是否已存在
    nvs_handle_t handle;
    size_t required_size = 0;
    ret = nvs_open(current_namespace, NVS_READWRITE, &handle);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "INSERT无法检查命名空间'%s':%s,若命名空间为空首次打开会失败,可以忽略", current_namespace, esp_err_to_name(ret));
    }
    ret = nvs_get_blob(handle, key, NULL, &required_size); 
    if (ret == ESP_OK) {
        ESP_LOGE(TAG, "INSERT字段'%s'已存在,无法插入.请使用UPDATE更新现有字段", key);
        return ESP_ERR_NVS_INVALID_STATE;
    } 
    // step2:进行写入
    ret = nvs_set_blob(handle, key, value, size);
    if (ret == ESP_OK) {
        ret = nvs_commit(handle);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "INSERT提交字段'%s'成功,数据大小:%dB", key, size);
        } 
        else {
            ESP_LOGE(TAG, "INSERT提交字段'%s'失败:%s", key, esp_err_to_name(ret));
        }
    } 
    else {
        ESP_LOGE(TAG, "INSERT插入字段'%s'失败:%s", key, esp_err_to_name(ret));
    }
    nvs_close(handle);
    ESP_LOGW(TAG,"INSERT成功");
    return ret;
}

/**
 * @brief 从NVS中删除数据
 * @param key 键名
 * @return ESP_OK 成功，其他值表示失败
 */
esp_err_t envs_delete(const char *key)
{
    ESP_LOGW(TAG,"DELETE FROM %s WHERE %s", current_namespace, key);
    esp_err_t ret = envs_validate_params(key, NULL, 0, true);
    if (ret != ESP_OK) {
        return ret;
    }
    nvs_handle_t handle;
    ret = nvs_open(current_namespace, NVS_READWRITE, &handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "DELETE无法打开命名空间'%s':%s", current_namespace, esp_err_to_name(ret));
        return ret;
    }
    ret = nvs_erase_key(handle, key);
    if (ret == ESP_OK) {
        ret = nvs_commit(handle);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "DELETE提交删除字段'%s'成功", key);
        } else {
            ESP_LOGE(TAG, "DELETE提交删除字段'%s'失败:%s", key, esp_err_to_name(ret));
        }
    } 
    else if (ret == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "DELETE字段'%s'不存在,删除失败", key);
        nvs_close(handle);
        return ret;
    } 
    else {
        ESP_LOGE(TAG, "DELETE删除字段'%s'失败:%s", key, esp_err_to_name(ret));
        nvs_close(handle);
        return ret;
    }
    nvs_close(handle);
    ESP_LOGW(TAG,"DELETE成功");
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

/**
 * @brief 测试envs的基本功能
 * @warning 本函数会擦除整个NVS
 * 
 */
void envs_utest(void)
{
    typedef struct{
        int a;
        int b;
        int c;
    }test_f;

    test_f out;
    test_f in = {1,2,3};

    envs_reset_all();
    envs_init();
    envs_use("TEST");


    ESP_LOGI(TAG,"首次读写测试");
    envs_insert("aa",&in,sizeof(test_f));
    envs_select("aa",&out,sizeof(test_f));
    ESP_LOGI(TAG,"1次写入,读到:%d-%d-%d",out.a,out.b,out.c);
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    ESP_LOGI(TAG,"多次d读写测试");
    ESP_LOGI(TAG,"多次读写测试");
    in.a = 999;
    envs_update("aa",&in,sizeof(test_f));
    envs_select("aa",&out,sizeof(test_f));
    ESP_LOGI(TAG,"2次写入,读到:%d-%d-%d",out.a,out.b,out.c);
    in.b = 999;
    envs_update("aa",&in,sizeof(test_f));
    envs_select("aa",&out,sizeof(test_f));
    ESP_LOGI(TAG,"3次写入,读到:%d-%d-%d",out.a,out.b,out.c);
    vTaskDelay(1000 / portTICK_PERIOD_MS);


    ESP_LOGI(TAG,"删除字段测试");
    envs_delete("aa");
    envs_select("aa",&out,sizeof(test_f));
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    ESP_LOGI(TAG,"插入存在的字段测试");
    envs_insert("aa","123",sizeof("123"));
    envs_insert("aa","123",sizeof("123"));
    vTaskDelay(1000 / portTICK_PERIOD_MS);


    ESP_LOGI(TAG,"读取不存在的字段测试");
    envs_select("bb",&out,sizeof(test_f));
    vTaskDelay(1000 / portTICK_PERIOD_MS);


    ESP_LOGI(TAG,"更新不存在的字段测试");
    envs_update("bb","123",sizeof("123"));
    vTaskDelay(1000 / portTICK_PERIOD_MS);


    ESP_LOGI(TAG,"删除不存在的字段测试");
    envs_delete("cc");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}