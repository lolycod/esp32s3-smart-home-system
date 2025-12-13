/**
 * @file servo_driver.h
 * @brief MG90S舵机PWM驱动
 *
 * 功能：
 * - 使用PWM控制MG90S舵机角度（0-180度）
 * - PWM频率：50Hz（周期20ms）
 * - 脉宽范围：500μs-2500μs
 *   - 500μs  → 0度
 *   - 1500μs → 90度
 *   - 2500μs → 180度
 */

#ifndef SERVO_DRIVER_H
#define SERVO_DRIVER_H

#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 舵机配置结构
 */
typedef struct {
    int gpio_pin;        ///< 舵机信号线GPIO
    uint32_t pwm_freq;   ///< PWM频率（Hz），标准50Hz
    uint16_t min_pulse;  ///< 最小脉宽（μs），默认500
    uint16_t max_pulse;  ///< 最大脉宽（μs），默认2500
} servo_config_t;

/**
 * @brief 初始化舵机驱动
 *
 * @param config 舵机配置
 * @return esp_err_t
 */
esp_err_t servo_init(const servo_config_t *config);

/**
 * @brief 设置舵机角度
 *
 * @param angle 角度 0-180度
 * @return esp_err_t
 */
esp_err_t servo_set_angle(uint8_t angle);

/**
 * @brief 设置舵机脉宽（高级用法）
 *
 * @param pulse_width_us 脉宽（微秒）500-2500
 * @return esp_err_t
 */
esp_err_t servo_set_pulse(uint16_t pulse_width_us);

/**
 * @brief 停止舵机输出（节能）
 *
 * @return esp_err_t
 */
esp_err_t servo_stop(void);

/**
 * @brief 反初始化舵机驱动
 *
 * @return esp_err_t
 */
esp_err_t servo_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // SERVO_DRIVER_H
