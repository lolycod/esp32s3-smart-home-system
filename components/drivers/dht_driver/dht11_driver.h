/**
 * @file dht11_driver.h
 * @brief DHT11温湿度传感器驱动层（ESP32平台）
 *
 * 这一层封装LibDriver的DHT11驱动，提供ESP32平台的适配
 * 职责：封装DHT11硬件操作，提供简洁的驱动接口
 *
 * @copyright Copyright (c) 2024
 */

#ifndef DHT11_DRIVER_H
#define DHT11_DRIVER_H

#include "esp_err.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief DHT11驱动配置
 */
typedef struct {
    uint8_t gpio_num;       ///< DHT11数据引脚
} dht11_driver_config_t;

/**
 * @brief DHT11读取的数据结构
 */
typedef struct {
    float temperature;      ///< 温度 (摄氏度)
    uint8_t humidity;       ///< 湿度 (%)
    uint16_t temperature_raw;  ///< 原始温度数据
    uint16_t humidity_raw;     ///< 原始湿度数据
} dht11_data_t;

/**
 * @brief 初始化DHT11驱动
 *
 * @param config DHT11配置
 * @return esp_err_t
 *         - ESP_OK: 成功
 *         - ESP_FAIL: 失败
 */
esp_err_t dht11_driver_init(const dht11_driver_config_t *config);

/**
 * @brief 读取温湿度数据
 *
 * @param data 输出参数，存储读取的数据
 * @return esp_err_t
 *         - ESP_OK: 成功
 *         - ESP_FAIL: 读取失败
 *         - ESP_ERR_TIMEOUT: 传感器无响应
 */
esp_err_t dht11_driver_read(dht11_data_t *data);

/**
 * @brief 反初始化DHT11驱动
 *
 * @return esp_err_t
 *         - ESP_OK: 成功
 */
esp_err_t dht11_driver_deinit(void);

/**
 * @brief 测试GPIO4基础功能（用于调试DHT11通信问题）
 *
 * 这个函数会测试GPIO4的基本读写功能，包括：
 * - 配置GPIO4为输入输出模式
 * - 测试高低电平切换
 * - 读取电平状态
 *
 * @return esp_err_t
 *         - ESP_OK: 测试通过
 *         - ESP_FAIL: 测试失败
 */
esp_err_t test_gpio4_basic(void);

#ifdef __cplusplus
}
#endif

#endif // DHT11_DRIVER_H
