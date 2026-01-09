/**
 * @file co2_driver.h
 * @brief JW01 二氧化碳传感器驱动头文件
 * 
 * JW01 是一款 NDIR CO2 传感器，通过 UART 通信
 * - 波特率: 9600
 * - 数据格式: 8N1
 * - 输出: CO2 浓度 (ppm)
 */

#ifndef CO2_DRIVER_H
#define CO2_DRIVER_H

#include "esp_err.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief CO2 传感器数据结构
 */
typedef struct {
    uint16_t co2_ppm;           ///< CO2 浓度 (ppm)
    uint16_t tvoc_ppb;          ///< TVOC 浓度 (ppb，可选)
    uint16_t ch2o_ppb;          ///< 甲醛浓度 (ppb，可选)
    bool valid;                 ///< 数据是否有效
} co2_data_t;

/**
 * @brief CO2 传感器配置结构
 */
typedef struct {
    int uart_tx_gpio;           ///< UART TX 引脚 (ESP32 -> 传感器)
    int uart_rx_gpio;           ///< UART RX 引脚 (传感器 -> ESP32)
    int uart_num;               ///< UART 端口号 (0, 1, 2)
} co2_driver_config_t;

/**
 * @brief 初始化 CO2 传感器驱动
 * 
 * @param config 驱动配置
 * @return esp_err_t ESP_OK 成功，其他值表示失败
 */
esp_err_t co2_driver_init(const co2_driver_config_t *config);

/**
 * @brief 读取 CO2 传感器数据
 * 
 * @param data 输出参数，存储读取的数据
 * @return esp_err_t ESP_OK 成功，其他值表示失败
 */
esp_err_t co2_driver_read(co2_data_t *data);

/**
 * @brief 反初始化 CO2 传感器驱动
 * 
 * @return esp_err_t ESP_OK 成功
 */
esp_err_t co2_driver_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // CO2_DRIVER_H
