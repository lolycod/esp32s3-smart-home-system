/**
 * @file rgb_pwm_driver.c
 * @brief 外接RGB灯PWM驱动实现
 */

#include "rgb_pwm_driver.h"
#include "driver/ledc.h"
#include "esp_log.h"

static const char *TAG = "RGB_PWM";

// PWM通道定义
#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL_R          LEDC_CHANNEL_0
#define LEDC_CHANNEL_G          LEDC_CHANNEL_1
#define LEDC_CHANNEL_B          LEDC_CHANNEL_2
#define LEDC_DUTY_RES           LEDC_TIMER_10_BIT  // 分辨率10位(0-1023)
#define LEDC_MAX_DUTY           (1023)  // 10位分辨率: 2^10 - 1 = 1023

// 存储GPIO配置
static int s_gpio_r = -1;
static int s_gpio_g = -1;
static int s_gpio_b = -1;
static bool s_initialized = false;

esp_err_t rgb_pwm_init(const rgb_pwm_config_t *config)
{
    if (config == NULL) {
        ESP_LOGE(TAG, "配置参数为空");
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "初始化RGB PWM驱动");
    ESP_LOGI(TAG, "  GPIO - R:%d, G:%d, B:%d", config->gpio_r, config->gpio_g, config->gpio_b);
    ESP_LOGI(TAG, "  PWM频率: %lu Hz", config->pwm_freq);

    // 保存GPIO配置
    s_gpio_r = config->gpio_r;
    s_gpio_g = config->gpio_g;
    s_gpio_b = config->gpio_b;

    // 配置定时器
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = config->pwm_freq,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // 配置红色通道
    ledc_channel_config_t ledc_channel_r = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL_R,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = s_gpio_r,
        .duty           = 0,
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_r));

    // 配置绿色通道
    ledc_channel_config_t ledc_channel_g = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL_G,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = s_gpio_g,
        .duty           = 0,
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_g));

    // 配置蓝色通道
    ledc_channel_config_t ledc_channel_b = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL_B,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = s_gpio_b,
        .duty           = 0,
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_b));

    s_initialized = true;
    ESP_LOGI(TAG, "✅ RGB PWM驱动初始化完成");
    return ESP_OK;
}

esp_err_t rgb_pwm_set_color(const rgb_color_t *color)
{
    if (!s_initialized) {
        ESP_LOGE(TAG, "驱动未初始化");
        return ESP_ERR_INVALID_STATE;
    }

    if (color == NULL) {
        ESP_LOGE(TAG, "颜色参数为空");
        return ESP_ERR_INVALID_ARG;
    }

    // 限制亮度范围
    uint8_t r = (color->r > 100) ? 100 : color->r;
    uint8_t g = (color->g > 100) ? 100 : color->g;
    uint8_t b = (color->b > 100) ? 100 : color->b;

    // 将亮度百分比转换为PWM占空比 (0-1023)
    uint32_t duty_r = (uint32_t)(r * LEDC_MAX_DUTY / 100);
    uint32_t duty_g = (uint32_t)(g * LEDC_MAX_DUTY / 100);
    uint32_t duty_b = (uint32_t)(b * LEDC_MAX_DUTY / 100);

    // 设置各通道占空比
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_R, duty_r));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_R));

    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_G, duty_g));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_G));

    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_B, duty_b));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_B));

    ESP_LOGI(TAG, "设置RGB颜色: R=%d%%, G=%d%%, B=%d%%", r, g, b);
    return ESP_OK;
}

esp_err_t rgb_pwm_set_channel(char channel, uint8_t brightness)
{
    if (!s_initialized) {
        ESP_LOGE(TAG, "驱动未初始化");
        return ESP_ERR_INVALID_STATE;
    }

    // 限制亮度范围
    if (brightness > 100) {
        brightness = 100;
    }

    // 转换为占空比
    uint32_t duty = (uint32_t)(brightness * LEDC_MAX_DUTY / 100);

    // 根据通道设置
    ledc_channel_t ledc_channel;
    switch (channel) {
        case 'r':
        case 'R':
            ledc_channel = LEDC_CHANNEL_R;
            break;
        case 'g':
        case 'G':
            ledc_channel = LEDC_CHANNEL_G;
            break;
        case 'b':
        case 'B':
            ledc_channel = LEDC_CHANNEL_B;
            break;
        default:
            ESP_LOGE(TAG, "无效的通道: %c", channel);
            return ESP_ERR_INVALID_ARG;
    }

    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, ledc_channel, duty));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, ledc_channel));

    ESP_LOGI(TAG, "设置通道%c亮度: %d%%", channel, brightness);
    return ESP_OK;
}

esp_err_t rgb_pwm_off(void)
{
    if (!s_initialized) {
        return ESP_OK;
    }

    rgb_color_t off = {0, 0, 0};
    return rgb_pwm_set_color(&off);
}

esp_err_t rgb_pwm_deinit(void)
{
    if (!s_initialized) {
        return ESP_OK;
    }

    // 关闭所有通道
    rgb_pwm_off();

    // 停止PWM通道
    ledc_stop(LEDC_MODE, LEDC_CHANNEL_R, 0);
    ledc_stop(LEDC_MODE, LEDC_CHANNEL_G, 0);
    ledc_stop(LEDC_MODE, LEDC_CHANNEL_B, 0);

    s_initialized = false;
    ESP_LOGI(TAG, "RGB PWM驱动已关闭");
    return ESP_OK;
}
