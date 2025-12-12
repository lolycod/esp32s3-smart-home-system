/**
 * @file tb6612_driver.c
 * @brief TB6612直流电机驱动实现
 *
 * 设计特点：
 * - 支持多电机实例（通过句柄管理）
 * - PWM集成在驱动层（无需单独的PWM HAL）
 * - 低耦合设计，易于扩展
 * - 完整的状态管理和错误处理
 */

#include "tb6612_driver.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <stdlib.h>
#include <string.h>

static const char *TAG = "TB6612_DRIVER";

/**
 * @brief 电机实例结构体
 */
typedef struct {
    tb6612_config_t config;      ///< 配置参数
    tb6612_state_t state;        ///< 当前状态
    tb6612_direction_t direction;///< 当前方向
    uint8_t speed;               ///< 当前速度 0-100
    bool initialized;            ///< 是否已初始化
} tb6612_motor_t;

// PWM相关常量
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES           LEDC_TIMER_10_BIT  // 10位分辨率(0-1023)
#define LEDC_MAX_DUTY           ((1 << LEDC_DUTY_RES) - 1)  // 1023

/**
 * @brief 初始化GPIO为输出模式
 */
static esp_err_t init_gpio_output(int gpio_num)
{
    if (gpio_num < 0) {
        return ESP_ERR_INVALID_ARG;
    }

    gpio_config_t io_conf = {0};
    io_conf.pin_bit_mask = (1ULL << gpio_num);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;

    return gpio_config(&io_conf);
}

/**
 * @brief 初始化LEDC PWM
 */
static esp_err_t init_ledc_pwm(const tb6612_config_t *config)
{
    // 配置LEDC定时器
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_MODE,
        .timer_num = config->ledc_timer,
        .duty_resolution = LEDC_DUTY_RES,
        .freq_hz = config->pwm_freq,
        .clk_cfg = LEDC_AUTO_CLK
    };

    esp_err_t ret = ledc_timer_config(&ledc_timer);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "LEDC定时器配置失败: %d", ret);
        return ret;
    }

    // 配置LEDC通道
    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_MODE,
        .channel = config->ledc_channel,
        .timer_sel = config->ledc_timer,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = config->gpio_pwm,
        .duty = 0,
        .hpoint = 0
    };

    ret = ledc_channel_config(&ledc_channel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "LEDC通道配置失败: %d", ret);
        return ret;
    }

    // 启动定时器
    ret = ledc_timer_resume(LEDC_MODE, config->ledc_timer);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "启动定时器失败: %d", ret);
        return ret;
    }

    return ESP_OK;
}

/**
 * @brief 设置PWM占空比
 */
static esp_err_t set_pwm_duty(tb6612_motor_t *motor, uint8_t speed)
{
    // 限制速度范围
    if (speed > 100) {
        speed = 100;
    }

    // 将百分比转换为占空比 (0-1023)
    uint32_t duty = (uint32_t)(speed * LEDC_MAX_DUTY / 100);

    esp_err_t ret = ledc_set_duty(LEDC_MODE, motor->config.ledc_channel, duty);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "设置占空比失败: %d", ret);
        return ret;
    }

    ret = ledc_update_duty(LEDC_MODE, motor->config.ledc_channel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "更新占空比失败: %d", ret);
        return ret;
    }

    motor->speed = speed;
    return ESP_OK;
}

/**
 * @brief 设置电机方向
 */
static esp_err_t set_direction(tb6612_motor_t *motor, tb6612_direction_t direction)
{
    if (direction == TB6612_DIR_FORWARD) {
        // 正转: IN1=1, IN2=0
        gpio_set_level(motor->config.gpio_in1, 1);
        gpio_set_level(motor->config.gpio_in2, 0);
        motor->direction = TB6612_DIR_FORWARD;
    } else {
        // 反转: IN1=0, IN2=1
        gpio_set_level(motor->config.gpio_in1, 0);
        gpio_set_level(motor->config.gpio_in2, 1);
        motor->direction = TB6612_DIR_BACKWARD;
    }

    return ESP_OK;
}

esp_err_t tb6612_create(const tb6612_config_t *config, tb6612_handle_t *handle)
{
    if (config == NULL || handle == NULL) {
        ESP_LOGE(TAG, "参数无效");
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "========== 初始化TB6612电机驱动 ==========");
    ESP_LOGI(TAG, "GPIO配置 - IN1:%d, IN2:%d, PWM:%d", 
             config->gpio_in1, config->gpio_in2, config->gpio_pwm);
    ESP_LOGI(TAG, "PWM频率: %lu Hz", config->pwm_freq);
    ESP_LOGI(TAG, "LEDC定时器: %d, 通道: %d", config->ledc_timer, config->ledc_channel);

    // 分配内存
    tb6612_motor_t *motor = (tb6612_motor_t *)malloc(sizeof(tb6612_motor_t));
    if (motor == NULL) {
        ESP_LOGE(TAG, "内存分配失败");
        return ESP_ERR_NO_MEM;
    }

    // 初始化结构体
    memset(motor, 0, sizeof(tb6612_motor_t));
    motor->config = *config;
    motor->state = TB6612_STATE_IDLE;
    motor->direction = TB6612_DIR_FORWARD;
    motor->speed = 0;

    // 初始化GPIO
    esp_err_t ret = init_gpio_output(config->gpio_in1);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO IN1初始化失败");
        free(motor);
        return ret;
    }

    ret = init_gpio_output(config->gpio_in2);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO IN2初始化失败");
        free(motor);
        return ret;
    }

    // 初始化PWM
    ret = init_ledc_pwm(config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "PWM初始化失败");
        free(motor);
        return ret;
    }

    // 初始化方向（默认正转）
    set_direction(motor, TB6612_DIR_FORWARD);

    // 初始化速度为0（停止）
    set_pwm_duty(motor, 0);

    motor->initialized = true;
    motor->state = TB6612_STATE_STOPPED;

    *handle = (tb6612_handle_t)motor;

    ESP_LOGI(TAG, "✅ TB6612电机驱动初始化完成");
    ESP_LOGI(TAG, "========================================");

    return ESP_OK;
}

esp_err_t tb6612_forward(tb6612_handle_t handle, uint8_t speed)
{
    if (handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    tb6612_motor_t *motor = (tb6612_motor_t *)handle;

    if (!motor->initialized) {
        ESP_LOGE(TAG, "电机未初始化");
        return ESP_ERR_INVALID_STATE;
    }

    // 设置方向
    set_direction(motor, TB6612_DIR_FORWARD);

    // 设置速度
    esp_err_t ret = set_pwm_duty(motor, speed);
    if (ret != ESP_OK) {
        return ret;
    }

    motor->state = TB6612_STATE_FORWARD;
    uint32_t actual_duty = ledc_get_duty(LEDC_MODE, motor->config.ledc_channel);
    ESP_LOGI(TAG, "电机→正转%d%% IN1=%d IN2=%d duty=%lu", speed,
             gpio_get_level(motor->config.gpio_in1),
             gpio_get_level(motor->config.gpio_in2),
             actual_duty);
    return ESP_OK;
}

esp_err_t tb6612_backward(tb6612_handle_t handle, uint8_t speed)
{
    if (handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    tb6612_motor_t *motor = (tb6612_motor_t *)handle;

    if (!motor->initialized) {
        ESP_LOGE(TAG, "电机未初始化");
        return ESP_ERR_INVALID_STATE;
    }

    // 设置方向
    set_direction(motor, TB6612_DIR_BACKWARD);

    // 设置速度
    esp_err_t ret = set_pwm_duty(motor, speed);
    if (ret != ESP_OK) {
        return ret;
    }

    motor->state = TB6612_STATE_BACKWARD;
    ESP_LOGI(TAG, "电机→反转%d%%", speed);
    return ESP_OK;
}

esp_err_t tb6612_stop(tb6612_handle_t handle)
{
    if (handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    tb6612_motor_t *motor = (tb6612_motor_t *)handle;

    if (!motor->initialized) {
        ESP_LOGE(TAG, "电机未初始化");
        return ESP_ERR_INVALID_STATE;
    }

    // 设置速度为0
    esp_err_t ret = set_pwm_duty(motor, 0);
    if (ret != ESP_OK) {
        return ret;
    }

    motor->state = TB6612_STATE_STOPPED;

    ESP_LOGI(TAG, "⏹️  电机已停止");
    return ESP_OK;
}

esp_err_t tb6612_set_speed(tb6612_handle_t handle, uint8_t speed)
{
    if (handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    tb6612_motor_t *motor = (tb6612_motor_t *)handle;

    if (!motor->initialized) {
        ESP_LOGE(TAG, "电机未初始化");
        return ESP_ERR_INVALID_STATE;
    }

    if (speed == 0) {
        return tb6612_stop(handle);
    }

    // 保持当前方向，只改变速度
    esp_err_t ret = set_pwm_duty(motor, speed);
    if (ret != ESP_OK) {
        return ret;
    }

    ESP_LOGI(TAG, "⚡ 电机速度调整 - 新速度: %d%%", speed);
    return ESP_OK;
}

tb6612_state_t tb6612_get_state(tb6612_handle_t handle)
{
    if (handle == NULL) {
        return TB6612_STATE_IDLE;
    }

    tb6612_motor_t *motor = (tb6612_motor_t *)handle;
    return motor->state;
}

uint8_t tb6612_get_speed(tb6612_handle_t handle)
{
    if (handle == NULL) {
        return 0;
    }

    tb6612_motor_t *motor = (tb6612_motor_t *)handle;
    return motor->speed;
}

tb6612_direction_t tb6612_get_direction(tb6612_handle_t handle)
{
    if (handle == NULL) {
        return TB6612_DIR_FORWARD;
    }

    tb6612_motor_t *motor = (tb6612_motor_t *)handle;
    return motor->direction;
}

esp_err_t tb6612_delete(tb6612_handle_t handle)
{
    if (handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    tb6612_motor_t *motor = (tb6612_motor_t *)handle;

    if (motor->initialized) {
        // 停止电机
        tb6612_stop(handle);

        // 停止PWM
        ledc_stop(LEDC_MODE, motor->config.ledc_channel, 0);

        motor->initialized = false;
    }

    free(motor);

    ESP_LOGI(TAG, "✅ 电机驱动已销毁");
    return ESP_OK;
}

