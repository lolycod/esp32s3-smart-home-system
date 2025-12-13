/**
 * @file wifi_service.h
 * @brief WiFi连接管理服务
 *
 * @copyright Copyright (c) 2024
 */

#ifndef WIFI_SERVICE_H
#define WIFI_SERVICE_H

#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief WiFi服务配置
 */
typedef struct {
    char ssid[32];          ///< WiFi SSID
    char password[64];      ///< WiFi密码
    uint8_t max_retry;      ///< 最大重连次数
} wifi_service_config_t;

/**
 * @brief 初始化WiFi服务
 *
 * @param config WiFi配置
 * @return esp_err_t
 */
esp_err_t wifi_service_init(const wifi_service_config_t *config);

/**
 * @brief 连接WiFi
 *
 * @return esp_err_t
 */
esp_err_t wifi_service_connect(void);

/**
 * @brief 断开WiFi连接
 *
 * @return esp_err_t
 */
esp_err_t wifi_service_disconnect(void);

/**
 * @brief 检查WiFi是否已连接
 *
 * @return true 已连接
 * @return false 未连接
 */
bool wifi_service_is_connected(void);

/**
 * @brief 反初始化WiFi服务
 *
 * @return esp_err_t
 */
esp_err_t wifi_service_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // WIFI_SERVICE_H
