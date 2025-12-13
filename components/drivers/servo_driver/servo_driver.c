/**
 * @file servo_driver.c
 * @brief MG90S舵机PWM驱动实现
 */

#include "servo_driver.h"
#include "driver/ledc.h"
#include "esp_log.h"

static const char *TAG = "SERVO_DRIVER";

// LEDC配置
#define LEDC_TIMER              LEDC_TIMER_1
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL            LEDC_CHANNEL_3
#define LEDC_DUTY_RES           LEDC_TIMER_14_BIT  // 14位分辨率
#define LEDC_MAX_DUTY           ((1 << LEDC_DUTY_RES) - 1)

// 舵机参数
static servo_config_t s_config;
static bool s_initialized = false;

/**
 * @brief 角度转换为占空比
 */
static uint32_t angle_to_duty(uint8_t angle)
{
    if (angle > 180) {
        angle = 180;
    }

    // 计算脉宽（微秒）
    uint16_t pulse_us = s_config.min_pulse +
                        (angle * (s_config.max_pulse - s_config.min_pulse)) / 180;

    // 周期 = 1000000 / 频率（微秒）
    uint32_t period_us = 1000000 / s_config.pwm_freq;

    // 占空比 = (脉宽 / 周期) * 最大占空比
    uint32_t duty = (pulse_us * LEDC_MAX_DUTY) / period_us;

    return duty;
}

/**
 * @brief 脉宽转换为占空比
 */
static uint32_t pulse_to_duty(uint16_t pulse_us)
{
    // 周期 = 1000000 / 频率（微秒）
    uint32_t period_us = 1000000 / s_config.pwm_freq;

    // 占空比 = (脉宽 / 周期) * 最大占空比
    uint32_t duty = (pulse_us * LEDC_MAX_DUTY) / period_us;

    return duty;
}

esp_err_t servo_init(const servo_config_t *config)
{
    if (config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    s_config = *config;

    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = config->pwm_freq,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    esp_err_t ret = ledc_timer_config(&ledc_timer);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "定时器配置失败:%d", ret);
        return ret;
    }

    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = config->gpio_pin,
        .duty           = 0,
        .hpoint         = 0
    };
    ret = ledc_channel_config(&ledc_channel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "通道配置失败:%d", ret);
        return ret;
    }

    // 启动定时器
    ret = ledc_timer_resume(LEDC_MODE, LEDC_TIMER);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "启动定时器失败:%d", ret);
        return ret;
    }

    s_initialized = true;
    ESP_LOGI(TAG, "舵机初始化完成 GPIO%d", config->gpio_pin);
    return ESP_OK;
}

esp_err_t servo_set_angle(uint8_t angle)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (angle > 180) angle = 180;

    uint32_t duty = angle_to_duty(angle);

    esp_err_t ret = ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "设置占空比失败:%d", ret);
        return ret;
    }

    ret = ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "更新占空比失败:%d", ret);
        return ret;
    }

    // 验证PWM是否真的输出
    uint32_t actual_duty = ledc_get_duty(LEDC_MODE, LEDC_CHANNEL);
    ESP_LOGI(TAG, "舵机→%d° duty=%lu/%d", angle, actual_duty, LEDC_MAX_DUTY);
    return ESP_OK;
}

esp_err_t servo_set_pulse(uint16_t pulse_width_us)
{
    if (!s_initialized) return ESP_ERR_INVALID_STATE;
    if (pulse_width_us < s_config.min_pulse || pulse_width_us > s_config.max_pulse) {
        return ESP_ERR_INVALID_ARG;
    }

    uint32_t duty = pulse_to_duty(pulse_width_us);
    esp_err_t ret = ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty);
    if (ret != ESP_OK) return ret;

    ret = ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
    return ret;
}

esp_err_t servo_stop(void)
{
    if (!s_initialized) return ESP_ERR_INVALID_STATE;
    return ledc_stop(LEDC_MODE, LEDC_CHANNEL, 0);
}

esp_err_t servo_deinit(void)
{
    if (!s_initialized) return ESP_OK;
    servo_stop();
    s_initialized = false;
    return ESP_OK;
}
