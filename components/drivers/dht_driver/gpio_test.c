/**
 * @file gpio_test.c
 * @brief 测试GPIO4是否正常工作
 */

#include "dht11_driver.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "GPIO_TEST";

/**
 * @brief 测试GPIO4能否正常读写
 */
esp_err_t test_gpio4_basic(void)
{
    ESP_LOGI(TAG, "========== 开始GPIO4基础测试 ==========");

    // 配置GPIO4为输出模式
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << 4),
        .mode = GPIO_MODE_INPUT_OUTPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO4配置失败!");
        return ret;
    }

    ESP_LOGI(TAG, "GPIO4配置成功");

    // 测试高低电平切换
    for (int i = 0; i < 5; i++) {
        gpio_set_level(4, 1);
        ESP_LOGI(TAG, "GPIO4设置为高电平");
        vTaskDelay(pdMS_TO_TICKS(500));

        int level = gpio_get_level(4);
        ESP_LOGI(TAG, "GPIO4读取电平: %d (应该是1)", level);

        gpio_set_level(4, 0);
        ESP_LOGI(TAG, "GPIO4设置为低电平");
        vTaskDelay(pdMS_TO_TICKS(500));

        level = gpio_get_level(4);
        ESP_LOGI(TAG, "GPIO4读取电平: %d (应该是0)", level);
    }

    ESP_LOGI(TAG, "========== GPIO4基础测试完成 ==========");
    return ESP_OK;
}
