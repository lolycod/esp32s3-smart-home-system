/**
 * @file mqtt_service.h
 * @brief MQTT云平台通信服务
 *
 * @copyright Copyright (c) 2024
 */

#ifndef MQTT_SERVICE_H
#define MQTT_SERVICE_H

#include "esp_err.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief MQTT服务配置
 */
typedef struct {
    char *broker_uri;       ///< MQTT Broker地址
    char *username;         ///< 用户名
    char *password;         ///< 密码
    char *client_id;        ///< 客户端ID
} mqtt_service_config_t;

/**
 * @brief 初始化MQTT服务
 *
 * @param config MQTT配置
 * @return esp_err_t
 */
esp_err_t mqtt_service_init(const mqtt_service_config_t *config);

/**
 * @brief 连接到MQTT Broker
 *
 * @return esp_err_t
 */
esp_err_t mqtt_service_connect(void);

/**
 * @brief 发布消息
 *
 * @param topic 主题
 * @param data 数据
 * @param len 数据长度
 * @return esp_err_t
 */
esp_err_t mqtt_service_publish(const char *topic, const char *data, int len);

/**
 * @brief 订阅主题
 *
 * @param topic 主题
 * @return esp_err_t
 */
esp_err_t mqtt_service_subscribe(const char *topic);

/**
 * @brief 断开MQTT连接
 *
 * @return esp_err_t
 */
esp_err_t mqtt_service_disconnect(void);

/**
 * @brief 反初始化MQTT服务
 *
 * @return esp_err_t
 */
esp_err_t mqtt_service_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // MQTT_SERVICE_H
