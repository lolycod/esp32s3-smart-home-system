/**
 * @file sensor_service.h
 * @brief 传感器业务逻辑服务层
 *
 * 职责：
 * - 管理传感器数据采集FreeRTOS任务
 * - 定时读取DHT11温湿度数据
 * - 通过队列将数据发送给应用层
 * - 支持多传感器扩展
 *
 * @copyright Copyright (c) 2024
 */

#ifndef SENSOR_SERVICE_H
#define SENSOR_SERVICE_H

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 传感器数据结构（可扩展）
 */
typedef struct {
    float temperature;          ///< 温度 (摄氏度)
    uint8_t humidity;           ///< 湿度 (%)
    bool smoke_detected;        ///< 烟雾检测状态
    float smoke_voltage;        ///< 烟雾传感器电压
    float light_intensity;      ///< 光照强度百分比 (0-100%)
    bool light_sufficient;      ///< 光照是否充足（来自DO）
    uint32_t timestamp_ms;      ///< 时间戳 (毫秒)
    bool valid;                 ///< 数据是否有效
} sensor_data_t;

/**
 * @brief 传感器服务配置
 */
typedef struct {
    uint8_t dht11_gpio;              ///< DHT11数据引脚
    int mq2_ttl_gpio;                ///< MQ-2 TTL输出引脚
    int mq2_adc_channel;             ///< MQ-2 ADC通道
    int ldr_do_gpio;                 ///< 5516光敏电阻DO引脚
    int ldr_adc_channel;             ///< 5516光敏电阻ADC通道
    uint32_t sample_interval_ms;     ///< 采样间隔(ms)
    QueueHandle_t data_queue;        ///< 数据输出队列
    const char *websocket_uri;       ///< WebSocket服务器URI (NULL则禁用)
} sensor_service_config_t;

/**
 * @brief 初始化传感器服务
 *
 * @param config 服务配置
 * @return esp_err_t
 *         - ESP_OK: 成功
 */
esp_err_t sensor_service_init(const sensor_service_config_t *config);

/**
 * @brief 启动传感器数据采集任务
 *
 * @return esp_err_t
 *         - ESP_OK: 成功
 */
esp_err_t sensor_service_start(void);

/**
 * @brief 停止传感器数据采集任务
 *
 * @return esp_err_t
 *         - ESP_OK: 成功
 */
esp_err_t sensor_service_stop(void);

/**
 * @brief 手动读取一次传感器数据（同步）
 *
 * @param data 输出参数，存储读取的数据
 * @return esp_err_t
 *         - ESP_OK: 成功
 */
esp_err_t sensor_service_read_once(sensor_data_t *data);

/**
 * @brief 反初始化传感器服务
 *
 * @return esp_err_t
 *         - ESP_OK: 成功
 */
esp_err_t sensor_service_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // SENSOR_SERVICE_H
