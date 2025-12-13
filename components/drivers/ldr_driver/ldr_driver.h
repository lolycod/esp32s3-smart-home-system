/**
 * @file ldr_driver.h
 * @brief 5516光敏电阻模块驱动头文件
 *
 * 功能：
 * - 使用ADC读取AO模拟输出（精确光照强度）
 * - 使用GPIO读取DO数字输出（阈值检测）
 * - 计算光照强度百分比
 * - 提供光照等级判断
 *
 * 硬件连接（5516模块）：
 * - VCC -> ESP32 3.3V
 * - GND -> ESP32 GND
 * - AO（模拟输出）-> GPIO7 (ADC1_CHANNEL_6)
 * - DO（数字输出）-> GPIO8 (低电平=光照不足)
 *
 * @copyright Copyright (c) 2024
 */

#ifndef LDR_DRIVER_H
#define LDR_DRIVER_H

#include "esp_err.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief 光照等级定义
 */
typedef enum {
    LIGHT_LEVEL_DARK = 0,      ///< 黑暗 (0-20%)
    LIGHT_LEVEL_DIM,           ///< 昏暗 (21-40%)
    LIGHT_LEVEL_MODERATE,      ///< 适中 (41-60%)
    LIGHT_LEVEL_BRIGHT,        ///< 明亮 (61-80%)
    LIGHT_LEVEL_VERY_BRIGHT,   ///< 非常明亮 (81-100%)
} light_level_t;

/**
 * @brief 5516光敏电阻模块配置
 */
typedef struct {
    int do_gpio;                ///< DO数字输出引脚（低电平=光照不足）
    int adc_channel;            ///< ADC通道（AO模拟输出，GPIO7=ADC1_CHANNEL_6）
    uint32_t sample_interval_ms; ///< 采样间隔（毫秒）
} ldr_driver_config_t;

/**
 * @brief 5516光敏电阻数据
 */
typedef struct {
    bool light_sufficient;      ///< 光照是否充足（DO输出，true=充足）
    uint32_t adc_value;         ///< ADC原始值（0-4095，从AO读取）
    float voltage;              ///< 电压值（0-3.3V）
    float light_intensity;      ///< 光照强度百分比（0-100%）
    light_level_t light_level;  ///< 光照等级
    uint32_t timestamp_ms;      ///< 时间戳
} ldr_data_t;

/**
 * @brief 初始化光敏电阻驱动
 *
 * @param config 配置参数
 * @return esp_err_t
 *         - ESP_OK: 成功
 *         - ESP_ERR_INVALID_ARG: 参数无效
 */
esp_err_t ldr_driver_init(const ldr_driver_config_t *config);

/**
 * @brief 读取光敏电阻数据
 *
 * @param data 存储读取的数据
 * @return esp_err_t
 *         - ESP_OK: 成功
 *         - ESP_ERR_INVALID_STATE: 驱动未初始化
 */
esp_err_t ldr_driver_read(ldr_data_t *data);

/**
 * @brief 反初始化光敏电阻驱动
 *
 * @return esp_err_t
 */
esp_err_t ldr_driver_deinit(void);

/**
 * @brief 获取光照等级描述字符串
 *
 * @param level 光照等级
 * @return const char* 描述字符串
 */
const char* ldr_get_level_string(light_level_t level);

#endif // LDR_DRIVER_H
