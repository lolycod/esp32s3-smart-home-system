/**
 * @file app_button.c
 * @brief 按键应用模块实现
 */

#include "app_button.h"
#include "esp_log.h"
#include "iot_button.h"
#include "button_gpio.h"

// 日志标签
static const char *TAG = "APP_BUTTON";

// 按键GPIO配置（根据你的硬件修改）
#define BUTTON_GPIO_NUM     0       // ESP32的BOOT按键通常在GPIO0
#define BUTTON_ACTIVE_LEVEL 0       // 0 = 低电平有效

// 按键句柄（全局变量，模块内部使用）
static button_handle_t s_btn_handle = NULL;

/**
 * @brief 单击事件回调
 */
static void button_single_click_cb(void *button_handle, void *usr_data)
{
    ESP_LOGI(TAG, "✓ 单击事件触发！");

    // TODO: 在这里添加你的单击处理逻辑
    // 例如：切换LED状态、发送消息等
}

/**
 * @brief 双击事件回调
 */
static void button_double_click_cb(void *button_handle, void *usr_data)
{
    ESP_LOGI(TAG, "✓✓ 双击事件触发！");

    // TODO: 在这里添加你的双击处理逻辑
}

/**
 * @brief 长按开始回调
 */
static void button_long_press_start_cb(void *button_handle, void *usr_data)
{
    ESP_LOGI(TAG, "⏱ 长按开始！");

    // TODO: 在这里添加你的长按处理逻辑
}

/**
 * @brief 长按保持回调
 */
static void button_long_press_hold_cb(void *button_handle, void *usr_data)
{
    uint32_t press_time = iot_button_get_ticks_time(button_handle);
    ESP_LOGI(TAG, "⏱ 长按保持中... 持续时间: %lu ms", press_time);
}

/**
 * @brief 初始化按键模块
 */
esp_err_t app_button_init(void)
{
    ESP_LOGI(TAG, "========== 初始化按键模块 ==========");

    // 1. 配置按键基本参数
    button_config_t btn_cfg = {
        .long_press_time = 1000,        // 长按触发时间：1000ms
        .short_press_time = 180,        // 短按去抖时间：180ms
    };

    // 2. 配置GPIO参数
    button_gpio_config_t gpio_btn_cfg = {
        .gpio_num = BUTTON_GPIO_NUM,
        .active_level = BUTTON_ACTIVE_LEVEL,
    };

    // 3. 创建按键设备
    esp_err_t ret = iot_button_new_gpio_device(&btn_cfg, &gpio_btn_cfg, &s_btn_handle);
    if (ret != ESP_OK || s_btn_handle == NULL) {
        ESP_LOGE(TAG, "❌ 按键创建失败！");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "✅ 按键创建成功！GPIO%d", BUTTON_GPIO_NUM);

    // 4. 注册事件回调
  iot_button_register_cb(s_btn_handle, BUTTON_SINGLE_CLICK, NULL, button_single_click_cb, NULL);
  iot_button_register_cb(s_btn_handle, BUTTON_DOUBLE_CLICK, NULL, button_double_click_cb, NULL);
  iot_button_register_cb(s_btn_handle, BUTTON_LONG_PRESS_START, NULL, button_long_press_start_cb, NULL);
  iot_button_register_cb(s_btn_handle, BUTTON_LONG_PRESS_HOLD, NULL, button_long_press_hold_cb, NULL);

    ESP_LOGI(TAG, "📌 按键事件已注册：");
    ESP_LOGI(TAG, "  - 单击：快速按下并释放");
    ESP_LOGI(TAG, "  - 双击：连续快速点击两次");
    ESP_LOGI(TAG, "  - 长按：按住不放超过1秒");
    ESP_LOGI(TAG, "========================================");

    return ESP_OK;
}

/**
 * @brief 反初始化按键模块
 */
esp_err_t app_button_deinit(void)
{
    if (s_btn_handle != NULL) {
        esp_err_t ret = iot_button_delete(s_btn_handle);
        if (ret == ESP_OK) {
            s_btn_handle = NULL;
            ESP_LOGI(TAG, "按键模块已释放");
            return ESP_OK;
        }
    }
    return ESP_FAIL;
}
