/**
 * @file button_service.h
 * @brief 按键业务逻辑服务层
 *
 * 这一层负责按键的业务逻辑处理，包括：
 * - 事件处理和分发
 * - 与其他服务的交互
 * - 业务状态管理
 *
 * @copyright Copyright (c) 2024
 */

#ifndef BUTTON_SERVICE_H
#define BUTTON_SERVICE_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 按键服务配置结构体
 */
typedef struct {
    uint8_t gpio_num;           ///< 按键GPIO引脚号
    uint8_t active_level;       ///< 有效电平
} button_service_config_t;

/**
 * @brief 初始化按键服务
 *
 * 这个函数会完成：
 * 1. 创建按键驱动实例
 * 2. 注册所有按键事件回调
 * 3. 启动按键检测
 *
 * @param config 按键服务配置
 * @return esp_err_t
 *         - ESP_OK: 成功
 *         - ESP_FAIL: 失败
 */
esp_err_t button_service_init(const button_service_config_t *config);

/**
 * @brief 反初始化按键服务
 *
 * @return esp_err_t
 *         - ESP_OK: 成功
 */
esp_err_t button_service_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // BUTTON_SERVICE_H
