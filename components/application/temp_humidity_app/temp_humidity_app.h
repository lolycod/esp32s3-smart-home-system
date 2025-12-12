/**
 * @file temp_humidity_app.h
 * @brief 温湿度应用层
 *
 * 职责：
 * - 接收传感器服务的数据
 * - 实现温湿度显示逻辑
 * - 管理应用层FreeRTOS任务
 * - 处理业务逻辑（数据展示、告警等）
 *
 * @copyright Copyright (c) 2024
 */

#ifndef TEMP_HUMIDITY_APP_H
#define TEMP_HUMIDITY_APP_H

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 温湿度应用配置
 */
typedef struct {
    QueueHandle_t sensor_data_queue;  ///< 传感器数据队列
} temp_humidity_app_config_t;

/**
 * @brief 初始化温湿度应用
 *
 * @param config 应用配置
 * @return esp_err_t
 *         - ESP_OK: 成功
 */
esp_err_t temp_humidity_app_init(const temp_humidity_app_config_t *config);

/**
 * @brief 启动温湿度应用任务
 *
 * @return esp_err_t
 *         - ESP_OK: 成功
 */
esp_err_t temp_humidity_app_start(void);

/**
 * @brief 停止温湿度应用任务
 *
 * @return esp_err_t
 *         - ESP_OK: 成功
 */
esp_err_t temp_humidity_app_stop(void);

/**
 * @brief 反初始化温湿度应用
 *
 * @return esp_err_t
 *         - ESP_OK: 成功
 */
esp_err_t temp_humidity_app_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // TEMP_HUMIDITY_APP_H
