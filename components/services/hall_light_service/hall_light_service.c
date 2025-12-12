/**
 * @file hall_light_service.c
 * @brief 大厅灯服务实现
 */

#include "hall_light_service.h"
#include "led_strip.h"
#include "esp_log.h"

static const char *TAG = "HALL_LIGHT";

// LED strip handle
static led_strip_handle_t s_led_strip = NULL;
static bool s_is_on = false;
static uint8_t s_brightness = 50;  // 默认50%亮度
static bool s_initialized = false;

// 固定颜色：绿色（大厅灯）
#define HALL_LIGHT_R    0
#define HALL_LIGHT_G    255
#define HALL_LIGHT_B    0

esp_err_t hall_light_init(const hall_light_config_t *config)
{
    if (config == NULL) {
        ESP_LOGE(TAG, "配置参数为空");
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "========== 初始化大厅灯服务 ==========");
    ESP_LOGI(TAG, "GPIO引脚: %d", config->gpio_pin);
    ESP_LOGI(TAG, "默认亮度: %d%%", config->brightness);

    s_brightness = config->brightness > 100 ? 100 : config->brightness;

    // 配置LED strip
    led_strip_config_t strip_config = {
        .strip_gpio_num = config->gpio_pin,
        .max_leds = 1,  // ESP32S3内置只有1个RGB LED
        .led_pixel_format = LED_PIXEL_FORMAT_GRB,
        .led_model = LED_MODEL_WS2812,
        .flags.invert_out = false,
    };

    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000,  // 10MHz
        .flags.with_dma = false,
    };

    esp_err_t ret = led_strip_new_rmt_device(&strip_config, &rmt_config, &s_led_strip);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "创建LED strip失败");
        return ret;
    }

    // 初始化为关闭状态
    led_strip_clear(s_led_strip);

    s_initialized = true;
    s_is_on = false;

    ESP_LOGI(TAG, "✅ 大厅灯服务初始化完成");
    ESP_LOGI(TAG, "========================================");
    return ESP_OK;
}

esp_err_t hall_light_on(void)
{
    if (!s_initialized) {
        ESP_LOGE(TAG, "服务未初始化");
        return ESP_ERR_INVALID_STATE;
    }

    // 根据亮度计算RGB值
    uint8_t r = (uint8_t)(HALL_LIGHT_R * s_brightness / 100);
    uint8_t g = (uint8_t)(HALL_LIGHT_G * s_brightness / 100);
    uint8_t b = (uint8_t)(HALL_LIGHT_B * s_brightness / 100);

    esp_err_t ret = led_strip_set_pixel(s_led_strip, 0, r, g, b);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = led_strip_refresh(s_led_strip);
    if (ret != ESP_OK) {
        return ret;
    }

    s_is_on = true;
    ESP_LOGI(TAG, "💡 大厅灯开启（亮度: %d%%）", s_brightness);
    return ESP_OK;
}

esp_err_t hall_light_off(void)
{
    if (!s_initialized) {
        ESP_LOGE(TAG, "服务未初始化");
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret = led_strip_clear(s_led_strip);
    if (ret != ESP_OK) {
        return ret;
    }

    s_is_on = false;
    ESP_LOGI(TAG, "🌑 大厅灯关闭");
    return ESP_OK;
}

esp_err_t hall_light_toggle(void)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (s_is_on) {
        return hall_light_off();
    } else {
        return hall_light_on();
    }
}

bool hall_light_is_on(void)
{
    return s_is_on;
}

esp_err_t hall_light_set_brightness(uint8_t brightness)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (brightness > 100) {
        brightness = 100;
    }

    s_brightness = brightness;
    ESP_LOGI(TAG, "设置亮度: %d%%", brightness);

    // 如果灯是开着的，更新亮度
    if (s_is_on) {
        return hall_light_on();
    }

    return ESP_OK;
}

esp_err_t hall_light_deinit(void)
{
    if (!s_initialized) {
        return ESP_OK;
    }

    hall_light_off();

    if (s_led_strip) {
        led_strip_del(s_led_strip);
        s_led_strip = NULL;
    }

    s_initialized = false;
    ESP_LOGI(TAG, "大厅灯服务已关闭");
    return ESP_OK;
}
