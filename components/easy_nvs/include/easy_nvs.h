/**
 * @file easy_nvs.h
 * @author CYK-Dot
 * @brief 将NVS操作封装成更易用的接口
 * @version 0.1
 * @date 2025-06-23
 *
 * @copyright Copyright (c) 2025
 */
#pragma once

/* 头文件引入 -----------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include <esp_err.h>

/* 配置宏定义 -----------------------------------------------------------------------*/

/* 导出宏定义 -----------------------------------------------------------------------*/

/* 导出类型定义 ---------------------------------------------------------------------*/

/* C++兼容 --------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

/* 导出函数声明 ----------------------------------------------------------------------*/

esp_err_t envs_init(void);
esp_err_t envs_use(const char *namespace);
esp_err_t envs_select(const char *key, void *out_value, size_t size);
esp_err_t envs_update(const char *key, const void *value, size_t size);
esp_err_t envs_insert(const char *key, const void *value, size_t size);
esp_err_t envs_delete(const char *key);

/* C++兼容 --------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif
