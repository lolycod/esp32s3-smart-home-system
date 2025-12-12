/**
 * @file led_driver.c
 * @brief LED驱动层实现
 */

#include "led_driver.h"
#include "gpio_hal.h"
#include "esp_log.h"

static const char *TAG = "LED_DRIVER";

static uint8_t s_led_pin = 0;
static bool s_active_level = true;
static bool s_current_state = false;

esp_err_t led_driver_init(const led_driver_config_t *config)
{
    if (config == NULL) {
        ESP_LOGE(TAG, "配置参数为空");
        return ESP_ERR_INVALID_ARG;
    }

    s_led_pin = config->gpio_num;
    s_active_level = config->active_level;

    // 使用GPIO HAL初始化
    gpio_hal_config_t gpio_cfg = {
        .pin = s_led_pin,
        .mode = GPIO_HAL_MODE_OUTPUT,
    };

    esp_err_t ret = gpio_hal_init(&gpio_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO初始化失败");
        return ret;
    }

    // 默认熄灭LED
    led_driver_set_state(false);

    ESP_LOGI(TAG, "LED驱动初始化成功，GPIO%d", s_led_pin);
    return ESP_OK;
}

esp_err_t led_driver_set_state(bool on)
{
    s_current_state = on;
    gpio_hal_level_t level = (on == s_active_level) ? GPIO_HAL_LEVEL_HIGH : GPIO_HAL_LEVEL_LOW;
    return gpio_hal_set_level(s_led_pin, level);
}

esp_err_t led_driver_toggle(void)
{
    return led_driver_set_state(!s_current_state);
}

esp_err_t led_driver_deinit(void)
{
    return gpio_hal_deinit(s_led_pin);
}
