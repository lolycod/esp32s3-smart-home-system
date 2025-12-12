/**
 * @file tb6612_driver.h
 * @brief TB6612直流电机驱动接口
 *
 * 功能：
 * - 使用TB6612驱动直流电机
 * - 支持正转、反转、停止、速度调节
 * - 支持多电机实例
 * - 低耦合设计，易于扩展
 *
 * TB6612引脚说明：
 * - IN1/IN2: 方向控制引脚（GPIO）
 * - PWM: 速度控制引脚（需要PWM输出）
 * - GND: 地
 * - VCC: 电源（通常5V）
 *
 * @copyright Copyright (c) 2024
 */

#ifndef TB6612_DRIVER_H
#define TB6612_DRIVER_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 电机运行状态
 */
typedef enum {
    TB6612_STATE_IDLE,      ///< 空闲（未初始化）
    TB6612_STATE_FORWARD,   ///< 正转
    TB6612_STATE_BACKWARD,  ///< 反转
    TB6612_STATE_STOPPED,   ///< 停止
} tb6612_state_t;

/**
 * @brief 电机运行方向
 */
typedef enum {
    TB6612_DIR_FORWARD = 0,   ///< 正转
    TB6612_DIR_BACKWARD = 1,  ///< 反转
} tb6612_direction_t;

/**
 * @brief TB6612电机驱动配置
 */
typedef struct {
    int gpio_in1;           ///< IN1引脚（方向控制1）
    int gpio_in2;           ///< IN2引脚（方向控制2）
    int gpio_pwm;           ///< PWM引脚（速度控制）
    uint32_t pwm_freq;      ///< PWM频率（Hz），建议5000-20000
    uint8_t ledc_timer;     ///< LEDC定时器编号（0-3）
    uint8_t ledc_channel;   ///< LEDC通道编号（0-7）
} tb6612_config_t;

/**
 * @brief TB6612电机驱动句柄
 */
typedef void* tb6612_handle_t;

/**
 * @brief 创建电机驱动实例
 *
 * @param config 电机配置
 * @param handle 输出的电机句柄
 * @return esp_err_t
 *         - ESP_OK: 成功
 *         - ESP_ERR_INVALID_ARG: 参数无效
 *         - ESP_ERR_NO_MEM: 内存不足
 */
esp_err_t tb6612_create(const tb6612_config_t *config, tb6612_handle_t *handle);

/**
 * @brief 电机正转
 *
 * @param handle 电机句柄
 * @param speed 速度 0-100（百分比）
 * @return esp_err_t
 */
esp_err_t tb6612_forward(tb6612_handle_t handle, uint8_t speed);

/**
 * @brief 电机反转
 *
 * @param handle 电机句柄
 * @param speed 速度 0-100（百分比）
 * @return esp_err_t
 */
esp_err_t tb6612_backward(tb6612_handle_t handle, uint8_t speed);

/**
 * @brief 停止电机
 *
 * @param handle 电机句柄
 * @return esp_err_t
 */
esp_err_t tb6612_stop(tb6612_handle_t handle);

/**
 * @brief 设置电机速度
 *
 * @param handle 电机句柄
 * @param speed 速度 0-100（百分比）
 * @return esp_err_t
 */
esp_err_t tb6612_set_speed(tb6612_handle_t handle, uint8_t speed);

/**
 * @brief 获取电机当前状态
 *
 * @param handle 电机句柄
 * @return tb6612_state_t 电机状态
 */
tb6612_state_t tb6612_get_state(tb6612_handle_t handle);

/**
 * @brief 获取电机当前速度
 *
 * @param handle 电机句柄
 * @return uint8_t 当前速度 0-100
 */
uint8_t tb6612_get_speed(tb6612_handle_t handle);

/**
 * @brief 获取电机当前方向
 *
 * @param handle 电机句柄
 * @return tb6612_direction_t 当前方向
 */
tb6612_direction_t tb6612_get_direction(tb6612_handle_t handle);

/**
 * @brief 销毁电机驱动实例
 *
 * @param handle 电机句柄
 * @return esp_err_t
 */
esp_err_t tb6612_delete(tb6612_handle_t handle);

#ifdef __cplusplus
}
#endif

#endif // TB6612_DRIVER_H

