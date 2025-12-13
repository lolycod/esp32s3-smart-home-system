/**
 * @file motor_app.h
 * @brief 电机应用层接口
 *
 * 功能：
 * - 提供电机应用示例
 * - 演示如何使用电机服务
 * - 包含测试函数
 *
 * @copyright Copyright (c) 2024
 */

#ifndef MOTOR_APP_H
#define MOTOR_APP_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化电机应用
 *
 * @return esp_err_t
 */
esp_err_t motor_app_init(void);

/**
 * @brief 电机基础功能测试
 *
 * 测试内容：
 * - 电机正转
 * - 电机反转
 * - 电机停止
 * - 速度调节
 *
 * @return esp_err_t
 */
esp_err_t motor_app_test_basic(void);

/**
 * @brief 电机速度渐进测试
 *
 * 测试内容：
 * - 从0%加速到100%
 * - 从100%减速到0%
 *
 * @return esp_err_t
 */
esp_err_t motor_app_test_speed_ramp(void);

/**
 * @brief 多电机协同测试
 *
 * 测试内容：
 * - 多个电机同时运行
 * - 不同电机不同速度
 *
 * @return esp_err_t
 */
esp_err_t motor_app_test_multi_motor(void);

/**
 * @brief 反初始化电机应用
 *
 * @return esp_err_t
 */
esp_err_t motor_app_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // MOTOR_APP_H

