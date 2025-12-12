/**
 * @file gpio_hal.c
 * @brief GPIO硬件抽象层实现（ESP32平台）
 */

#include "gpio_hal.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "GPIO_HAL";

/**
 * @brief 初始化GPIO引脚
 */
esp_err_t gpio_hal_init(const gpio_hal_config_t *config)
{
    if (config == NULL) {
        ESP_LOGE(TAG, "配置参数为空");
        return ESP_ERR_INVALID_ARG;
    }

    gpio_config_t io_conf = {0};
    io_conf.pin_bit_mask = (1ULL << config->pin);
    io_conf.intr_type = GPIO_INTR_DISABLE;

    // 根据模式配置GPIO
    switch (config->mode) {
        case GPIO_HAL_MODE_INPUT:
            io_conf.mode = GPIO_MODE_INPUT;
            io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
            io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
            break;

        case GPIO_HAL_MODE_OUTPUT:
            io_conf.mode = GPIO_MODE_OUTPUT;
            io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
            io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
            break;

        case GPIO_HAL_MODE_INPUT_PULLUP:
            io_conf.mode = GPIO_MODE_INPUT;
            io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
            io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
            break;

        case GPIO_HAL_MODE_INPUT_PULLDOWN:
            io_conf.mode = GPIO_MODE_INPUT;
            io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
            io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
            break;

        default:
            ESP_LOGE(TAG, "未知的GPIO模式: %d", config->mode);
            return ESP_ERR_INVALID_ARG;
    }

    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO%d配置失败", config->pin);
        return ret;
    }

    ESP_LOGD(TAG, "GPIO%d初始化成功，模式=%d", config->pin, config->mode);
    return ESP_OK;
}

/**
 * @brief 设置GPIO输出电平
 */
esp_err_t gpio_hal_set_level(uint8_t pin, gpio_hal_level_t level)
{
    return gpio_set_level(pin, level);
}

/**
 * @brief 读取GPIO输入电平
 */
gpio_hal_level_t gpio_hal_get_level(uint8_t pin)
{
    return (gpio_hal_level_t)gpio_get_level(pin);
}

/**
 * @brief 反初始化GPIO引脚
 */
esp_err_t gpio_hal_deinit(uint8_t pin)
{
    return gpio_reset_pin(pin);
}
