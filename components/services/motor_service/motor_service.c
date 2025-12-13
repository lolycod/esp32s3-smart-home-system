/**
 * @file motor_service.c
 * @brief 电机服务层实现
 *
 * 职责：
 * - 管理多个电机实例
 * - 提供统一的电机控制接口
 * - 处理电机状态管理
 * - 简化应用层调用
 */

#include "motor_service.h"
#include "tb6612_driver.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "MOTOR_SERVICE";

/**
 * @brief 电机服务全局状态
 */
typedef struct {
    tb6612_handle_t motors[MOTOR_ID_MAX];  ///< 电机驱动句柄
    uint8_t motor_count;                   ///< 实际电机数量
    bool initialized;                      ///< 是否已初始化
} motor_service_state_t;

static motor_service_state_t s_motor_service = {0};

/**
 * @brief 验证电机ID有效性
 */
static bool is_valid_motor_id(motor_id_t motor_id)
{
    return (motor_id >= 0 && motor_id < s_motor_service.motor_count);
}

esp_err_t motor_service_init(const motor_service_config_t *config)
{
    if (config == NULL || config->motor_count == 0 || config->motor_count > MOTOR_ID_MAX) {
        ESP_LOGE(TAG, "配置参数无效");
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "========== 初始化电机服务 ==========");
    ESP_LOGI(TAG, "电机数量: %d", config->motor_count);

    // 初始化全局状态
    memset(&s_motor_service, 0, sizeof(motor_service_state_t));
    s_motor_service.motor_count = config->motor_count;

    // 创建各电机驱动实例
    for (uint8_t i = 0; i < config->motor_count; i++) {
        ESP_LOGI(TAG, "");
        ESP_LOGI(TAG, "初始化电机 %d...", i + 1);

        // 转换配置格式
        tb6612_config_t tb6612_cfg = {
            .gpio_in1 = config->motors[i].gpio_in1,
            .gpio_in2 = config->motors[i].gpio_in2,
            .gpio_pwm = config->motors[i].gpio_pwm,
            .pwm_freq = config->motors[i].pwm_freq,
            .ledc_timer = config->motors[i].ledc_timer,
            .ledc_channel = config->motors[i].ledc_channel,
        };

        // 创建电机驱动
        esp_err_t ret = tb6612_create(&tb6612_cfg, &s_motor_service.motors[i]);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "❌ 电机 %d 初始化失败", i + 1);
            // 清理已初始化的电机
            for (uint8_t j = 0; j < i; j++) {
                tb6612_delete(s_motor_service.motors[j]);
            }
            return ret;
        }

        ESP_LOGI(TAG, "✅ 电机 %d 初始化成功", i + 1);
    }

    s_motor_service.initialized = true;

    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "✅ 电机服务初始化完成");
    ESP_LOGI(TAG, "========================================");

    return ESP_OK;
}

esp_err_t motor_service_forward(motor_id_t motor_id, uint8_t speed)
{
    if (!s_motor_service.initialized) {
        ESP_LOGE(TAG, "服务未初始化");
        return ESP_ERR_INVALID_STATE;
    }

    if (!is_valid_motor_id(motor_id)) {
        ESP_LOGE(TAG, "无效的电机ID: %d", motor_id);
        return ESP_ERR_INVALID_ARG;
    }

    return tb6612_forward(s_motor_service.motors[motor_id], speed);
}

esp_err_t motor_service_backward(motor_id_t motor_id, uint8_t speed)
{
    if (!s_motor_service.initialized) {
        ESP_LOGE(TAG, "服务未初始化");
        return ESP_ERR_INVALID_STATE;
    }

    if (!is_valid_motor_id(motor_id)) {
        ESP_LOGE(TAG, "无效的电机ID: %d", motor_id);
        return ESP_ERR_INVALID_ARG;
    }

    return tb6612_backward(s_motor_service.motors[motor_id], speed);
}

esp_err_t motor_service_stop(motor_id_t motor_id)
{
    if (!s_motor_service.initialized) {
        ESP_LOGE(TAG, "服务未初始化");
        return ESP_ERR_INVALID_STATE;
    }

    if (!is_valid_motor_id(motor_id)) {
        ESP_LOGE(TAG, "无效的电机ID: %d", motor_id);
        return ESP_ERR_INVALID_ARG;
    }

    return tb6612_stop(s_motor_service.motors[motor_id]);
}

esp_err_t motor_service_set_speed(motor_id_t motor_id, uint8_t speed)
{
    if (!s_motor_service.initialized) {
        ESP_LOGE(TAG, "服务未初始化");
        return ESP_ERR_INVALID_STATE;
    }

    if (!is_valid_motor_id(motor_id)) {
        ESP_LOGE(TAG, "无效的电机ID: %d", motor_id);
        return ESP_ERR_INVALID_ARG;
    }

    return tb6612_set_speed(s_motor_service.motors[motor_id], speed);
}

motor_state_t motor_service_get_state(motor_id_t motor_id)
{
    if (!s_motor_service.initialized || !is_valid_motor_id(motor_id)) {
        return MOTOR_STATE_IDLE;
    }

    // 转换状态类型
    tb6612_state_t tb6612_state = tb6612_get_state(s_motor_service.motors[motor_id]);
    return (motor_state_t)tb6612_state;
}

uint8_t motor_service_get_speed(motor_id_t motor_id)
{
    if (!s_motor_service.initialized || !is_valid_motor_id(motor_id)) {
        return 0;
    }

    return tb6612_get_speed(s_motor_service.motors[motor_id]);
}

motor_direction_t motor_service_get_direction(motor_id_t motor_id)
{
    if (!s_motor_service.initialized || !is_valid_motor_id(motor_id)) {
        return MOTOR_DIR_FORWARD;
    }

    // 转换方向类型
    tb6612_direction_t tb6612_dir = tb6612_get_direction(s_motor_service.motors[motor_id]);
    return (motor_direction_t)tb6612_dir;
}

esp_err_t motor_service_stop_all(void)
{
    if (!s_motor_service.initialized) {
        ESP_LOGE(TAG, "服务未初始化");
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "停止所有电机...");

    for (uint8_t i = 0; i < s_motor_service.motor_count; i++) {
        tb6612_stop(s_motor_service.motors[i]);
    }

    ESP_LOGI(TAG, "✅ 所有电机已停止");
    return ESP_OK;
}

esp_err_t motor_service_deinit(void)
{
    if (!s_motor_service.initialized) {
        return ESP_OK;
    }

    ESP_LOGI(TAG, "反初始化电机服务...");

    // 停止所有电机
    motor_service_stop_all();

    // 销毁所有电机驱动
    for (uint8_t i = 0; i < s_motor_service.motor_count; i++) {
        tb6612_delete(s_motor_service.motors[i]);
    }

    memset(&s_motor_service, 0, sizeof(motor_service_state_t));

    ESP_LOGI(TAG, "✅ 电机服务已关闭");
    return ESP_OK;
}

