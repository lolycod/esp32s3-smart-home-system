/**
 * @file mq2_driver.h
 * @brief MQ-2烟雾传感器驱动头文件
 */

#ifndef MQ2_DRIVER_H
#define MQ2_DRIVER_H

#include "esp_err.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief MQ-2传感器配置
 */
typedef struct {
    int ttl_gpio;           // TTL数字输出引脚（低电平=检测到烟雾）
    int adc_channel;        // ADC通道（用于模拟量读取）
    uint32_t sample_interval_ms;  // 采样间隔（毫秒）
} mq2_driver_config_t;

/**
 * @brief MQ-2传感器数据
 */
typedef struct {
    bool smoke_detected;    // 烟雾检测状态（true=检测到）
    uint32_t adc_value;     // ADC原始值（0-4095）
    float voltage;          // 电压值（0-3.3V）
    float concentration;    // 烟雾浓度百分比（0-100%）
    uint32_t timestamp_ms;  // 时间戳
} mq2_data_t;

/**
 * @brief 初始化MQ-2驱动
 */
esp_err_t mq2_driver_init(const mq2_driver_config_t *config);

/**
 * @brief 读取MQ-2传感器数据
 */
esp_err_t mq2_driver_read(mq2_data_t *data);

/**
 * @brief 反初始化MQ-2驱动
 */
esp_err_t mq2_driver_deinit(void);

#endif // MQ2_DRIVER_H
