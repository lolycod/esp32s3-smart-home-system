/**
 * @file motor_service.h
 * @brief 电机服务层接口
 *
 * 功能：
 * - 提供高级电机管理接口
 * - 支持多电机管理
 * - 提供电机状态查询
 * - 简化应用层调用
 *
 * 设计理念：
 * - 服务层在驱动层之上，提供更高级的抽象
 * - 应用层通过服务层控制电机，而不是直接调用驱动层
 * - 便于后续添加电机管理功能（如加速度控制、位置反馈等）
 *
 * @copyright Copyright (c) 2024
 */

#ifndef MOTOR_SERVICE_H
#define MOTOR_SERVICE_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 电机ID定义（支持最多4个电机）
 */
typedef enum {
    MOTOR_ID_1 = 0,  ///< 电机1
    MOTOR_ID_2 = 1,  ///< 电机2
    MOTOR_ID_3 = 2,  ///< 电机3
    MOTOR_ID_4 = 3,  ///< 电机4
    MOTOR_ID_MAX = 4 ///< 最大电机数
} motor_id_t;

/**
 * @brief 电机运行状态
 */
typedef enum {
    MOTOR_STATE_IDLE,      ///< 空闲
    MOTOR_STATE_FORWARD,   ///< 正转
    MOTOR_STATE_BACKWARD,  ///< 反转
    MOTOR_STATE_STOPPED,   ///< 停止
} motor_state_t;

/**
 * @brief 电机运行方向
 */
typedef enum {
    MOTOR_DIR_FORWARD = 0,   ///< 正转
    MOTOR_DIR_BACKWARD = 1,  ///< 反转
} motor_direction_t;

/**
 * @brief 单个电机配置
 */
typedef struct {
    int gpio_in1;           ///< IN1引脚
    int gpio_in2;           ///< IN2引脚
    int gpio_pwm;           ///< PWM引脚
    uint32_t pwm_freq;      ///< PWM频率
    uint8_t ledc_timer;     ///< LEDC定时器
    uint8_t ledc_channel;   ///< LEDC通道
} motor_config_t;

/**
 * @brief 电机服务配置
 */
typedef struct {
    motor_config_t motors[MOTOR_ID_MAX];  ///< 各电机配置
    uint8_t motor_count;                  ///< 实际电机数量
} motor_service_config_t;

/**
 * @brief 初始化电机服务
 *
 * @param config 服务配置
 * @return esp_err_t
 *         - ESP_OK: 成功
 *         - ESP_ERR_INVALID_ARG: 参数无效
 */
esp_err_t motor_service_init(const motor_service_config_t *config);

/**
 * @brief 电机正转
 *
 * @param motor_id 电机ID
 * @param speed 速度 0-100（百分比）
 * @return esp_err_t
 */
esp_err_t motor_service_forward(motor_id_t motor_id, uint8_t speed);

/**
 * @brief 电机反转
 *
 * @param motor_id 电机ID
 * @param speed 速度 0-100（百分比）
 * @return esp_err_t
 */
esp_err_t motor_service_backward(motor_id_t motor_id, uint8_t speed);

/**
 * @brief 停止电机
 *
 * @param motor_id 电机ID
 * @return esp_err_t
 */
esp_err_t motor_service_stop(motor_id_t motor_id);

/**
 * @brief 设置电机速度
 *
 * @param motor_id 电机ID
 * @param speed 速度 0-100（百分比）
 * @return esp_err_t
 */
esp_err_t motor_service_set_speed(motor_id_t motor_id, uint8_t speed);

/**
 * @brief 获取电机状态
 *
 * @param motor_id 电机ID
 * @return motor_state_t 电机状态
 */
motor_state_t motor_service_get_state(motor_id_t motor_id);

/**
 * @brief 获取电机速度
 *
 * @param motor_id 电机ID
 * @return uint8_t 当前速度 0-100
 */
uint8_t motor_service_get_speed(motor_id_t motor_id);

/**
 * @brief 获取电机方向
 *
 * @param motor_id 电机ID
 * @return motor_direction_t 当前方向
 */
motor_direction_t motor_service_get_direction(motor_id_t motor_id);

/**
 * @brief 停止所有电机
 *
 * @return esp_err_t
 */
esp_err_t motor_service_stop_all(void);

/**
 * @brief 反初始化电机服务
 *
 * @return esp_err_t
 */
esp_err_t motor_service_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // MOTOR_SERVICE_H

