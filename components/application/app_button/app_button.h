/**
 * @file app_button.h
 * @brief 按键应用模块头文件
 *
 * 这个模块负责按键的初始化和事件处理
 */

#ifndef APP_BUTTON_H
#define APP_BUTTON_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化按键模块
 *
 * 这个函数会：
 * 1. 创建GPIO按键设备
 * 2. 注册所有按键事件回调
 * 3. 启动按键检测
 *
 * @return
 *     - ESP_OK: 初始化成功
 *     - ESP_FAIL: 初始化失败
 */
esp_err_t app_button_init(void);

/**
 * @brief 反初始化按键模块
 *
 * @return
 *     - ESP_OK: 成功
 *     - ESP_FAIL: 失败
 */
esp_err_t app_button_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // APP_BUTTON_H
