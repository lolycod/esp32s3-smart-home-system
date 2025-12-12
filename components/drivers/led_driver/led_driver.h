/**
 * @file led_driver.h
 * @brief LED驱动层接口
 *
 * @copyright Copyright (c) 2024
 */

#ifndef LED_DRIVER_H
#define LED_DRIVER_H

#include "esp_err.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief LED驱动配置
 */
typedef struct {
    uint8_t gpio_num;       ///< LED连接的GPIO引脚
    bool active_level;      ///< 有效电平 (true=高电平点亮, false=低电平点亮)
} led_driver_config_t;

/**
 * @brief 初始化LED驱动
 *
 * @param config LED配置
 * @return esp_err_t
 */
esp_err_t led_driver_init(const led_driver_config_t *config);

/**
 * @brief 设置LED状态
 *
 * @param on true=点亮, false=熄灭
 * @return esp_err_t
 */
esp_err_t led_driver_set_state(bool on);

/**
 * @brief 切换LED状态
 *
 * @return esp_err_t
 */
esp_err_t led_driver_toggle(void);

/**
 * @brief 反初始化LED驱动
 *
 * @return esp_err_t
 */
esp_err_t led_driver_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // LED_DRIVER_H
