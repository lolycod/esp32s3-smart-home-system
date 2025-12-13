/**
 * @file gpio_hal.h
 * @brief GPIO硬件抽象层接口
 *
 * 这一层封装了ESP32的GPIO底层操作，提供统一的硬件抽象接口。
 * 如果将来移植到其他平台（如STM32），只需修改这一层的实现。
 *
 * @copyright Copyright (c) 2024
 */

#ifndef GPIO_HAL_H
#define GPIO_HAL_H

#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief GPIO电平定义
 */
typedef enum {
    GPIO_HAL_LEVEL_LOW = 0,     ///< 低电平
    GPIO_HAL_LEVEL_HIGH = 1     ///< 高电平
} gpio_hal_level_t;

/**
 * @brief GPIO模式定义
 */
typedef enum {
    GPIO_HAL_MODE_INPUT = 0,        ///< 输入模式
    GPIO_HAL_MODE_OUTPUT,           ///< 输出模式
    GPIO_HAL_MODE_INPUT_PULLUP,     ///< 输入上拉模式
    GPIO_HAL_MODE_INPUT_PULLDOWN,   ///< 输入下拉模式
} gpio_hal_mode_t;

/**
 * @brief GPIO配置结构体
 */
typedef struct {
    uint8_t pin;                ///< GPIO引脚号
    gpio_hal_mode_t mode;       ///< GPIO模式
} gpio_hal_config_t;

/**
 * @brief 初始化GPIO引脚
 *
 * @param config GPIO配置结构体指针
 * @return esp_err_t
 *         - ESP_OK: 成功
 *         - ESP_ERR_INVALID_ARG: 参数无效
 */
esp_err_t gpio_hal_init(const gpio_hal_config_t *config);

/**
 * @brief 设置GPIO输出电平
 *
 * @param pin GPIO引脚号
 * @param level 电平值
 * @return esp_err_t
 *         - ESP_OK: 成功
 */
esp_err_t gpio_hal_set_level(uint8_t pin, gpio_hal_level_t level);

/**
 * @brief 读取GPIO输入电平
 *
 * @param pin GPIO引脚号
 * @return gpio_hal_level_t 当前电平
 */
gpio_hal_level_t gpio_hal_get_level(uint8_t pin);

/**
 * @brief 反初始化GPIO引脚
 *
 * @param pin GPIO引脚号
 * @return esp_err_t
 *         - ESP_OK: 成功
 */
esp_err_t gpio_hal_deinit(uint8_t pin);

#ifdef __cplusplus
}
#endif

#endif // GPIO_HAL_H
