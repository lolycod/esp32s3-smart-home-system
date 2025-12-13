/**
 * @file button_driver.h
 * @brief 按键驱动层接口
 *
 * 这一层封装按键的底层驱动，负责按键的创建、配置和底层事件检测。
 * 不包含业务逻辑，只提供标准的驱动接口。
 *
 * @copyright Copyright (c) 2024
 */

#ifndef BUTTON_DRIVER_H
#define BUTTON_DRIVER_H

#include "esp_err.h"
#include "iot_button.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 按键驱动配置结构体
 */
typedef struct {
    uint8_t gpio_num;           ///< GPIO引脚号
    uint8_t active_level;       ///< 有效电平 (0=低电平有效, 1=高电平有效)
    uint16_t long_press_time;   ///< 长按触发时间(ms)
    uint16_t short_press_time;  ///< 短按去抖时间(ms)
} button_driver_config_t;

/**
 * @brief 按键句柄类型定义
 */
typedef button_handle_t button_driver_handle_t;

/**
 * @brief 按键事件类型（映射到iot_button的事件）
 */
typedef button_event_t button_driver_event_t;

/**
 * @brief 按键事件回调函数类型
 */
typedef void (*button_driver_callback_t)(void *handle, void *user_data);

/**
 * @brief 创建按键设备
 *
 * @param config 按键配置结构体指针
 * @param handle 输出参数，返回按键句柄
 * @return esp_err_t
 *         - ESP_OK: 成功
 *         - ESP_ERR_INVALID_ARG: 参数无效
 *         - ESP_FAIL: 创建失败
 */
esp_err_t button_driver_create(const button_driver_config_t *config, button_driver_handle_t *handle);

/**
 * @brief 注册按键事件回调
 *
 * @param handle 按键句柄
 * @param event 事件类型
 * @param callback 回调函数
 * @param user_data 用户数据指针
 * @return esp_err_t
 *         - ESP_OK: 成功
 */
esp_err_t button_driver_register_callback(
    button_driver_handle_t handle,
    button_driver_event_t event,
    button_driver_callback_t callback,
    void *user_data
);

/**
 * @brief 获取按键按下时长
 *
 * @param handle 按键句柄
 * @return uint32_t 按下时长(ms)
 */
uint32_t button_driver_get_press_time(button_driver_handle_t handle);

/**
 * @brief 删除按键设备
 *
 * @param handle 按键句柄
 * @return esp_err_t
 *         - ESP_OK: 成功
 */
esp_err_t button_driver_delete(button_driver_handle_t handle);

#ifdef __cplusplus
}
#endif

#endif // BUTTON_DRIVER_H
