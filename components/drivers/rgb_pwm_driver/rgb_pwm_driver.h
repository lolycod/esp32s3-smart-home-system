/**
 * @file rgb_pwm_driver.h
 * @brief 外接RGB灯PWM驱动（用于空调压缩机模拟）
 *
 * 功能：
 * - 使用PWM控制外接RGB灯的红、绿、蓝三个通道
 * - 支持设置各通道的亮度（0-100%）
 * - 用于模拟空调压缩机状态：
 *   绿色：舒适状态
 *   红色深度：制热强度
 *   蓝色深度：制冷强度
 */

#ifndef RGB_PWM_DRIVER_H
#define RGB_PWM_DRIVER_H

#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief RGB灯配置结构
 */
typedef struct {
    int gpio_r;          ///< 红色通道GPIO
    int gpio_g;          ///< 绿色通道GPIO
    int gpio_b;          ///< 蓝色通道GPIO
    uint32_t pwm_freq;   ///< PWM频率（Hz），建议5000
} rgb_pwm_config_t;

/**
 * @brief RGB颜色结构（亮度0-100）
 */
typedef struct {
    uint8_t r;  ///< 红色亮度 0-100
    uint8_t g;  ///< 绿色亮度 0-100
    uint8_t b;  ///< 蓝色亮度 0-100
} rgb_color_t;

/**
 * @brief 初始化RGB PWM驱动
 *
 * @param config RGB灯配置
 * @return esp_err_t
 */
esp_err_t rgb_pwm_init(const rgb_pwm_config_t *config);

/**
 * @brief 设置RGB颜色
 *
 * @param color RGB颜色（亮度0-100）
 * @return esp_err_t
 */
esp_err_t rgb_pwm_set_color(const rgb_color_t *color);

/**
 * @brief 设置单个通道亮度
 *
 * @param channel 通道：'r', 'g', 'b'
 * @param brightness 亮度 0-100
 * @return esp_err_t
 */
esp_err_t rgb_pwm_set_channel(char channel, uint8_t brightness);

/**
 * @brief 关闭所有通道
 *
 * @return esp_err_t
 */
esp_err_t rgb_pwm_off(void);

/**
 * @brief 反初始化RGB PWM驱动
 *
 * @return esp_err_t
 */
esp_err_t rgb_pwm_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // RGB_PWM_DRIVER_H
