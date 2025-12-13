/**
 * @file button_driver.c
 * @brief 按键驱动层实现
 */

#include "button_driver.h"
#include "esp_log.h"

static const char *TAG = "BUTTON_DRIVER";

/**
 * @brief 创建按键设备
 */
esp_err_t button_driver_create(const button_driver_config_t *config, button_driver_handle_t *handle)
{
    if (config == NULL || handle == NULL) {
        ESP_LOGE(TAG, "参数无效");
        return ESP_ERR_INVALID_ARG;
    }

    // 配置按键基本参数
    button_config_t btn_cfg = {
        .long_press_time = config->long_press_time,
        .short_press_time = config->short_press_time,
    };

    // 配置GPIO参数
    button_gpio_config_t gpio_btn_cfg = {
        .gpio_num = config->gpio_num,
        .active_level = config->active_level,
    };

    // 创建按键设备
    esp_err_t ret = iot_button_new_gpio_device(&btn_cfg, &gpio_btn_cfg, handle);
    if (ret != ESP_OK || *handle == NULL) {
        ESP_LOGE(TAG, "按键创建失败，GPIO=%d", config->gpio_num);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "按键设备创建成功: GPIO%d, 有效电平=%d, 长按时间=%dms",
             config->gpio_num, config->active_level, config->long_press_time);

    return ESP_OK;
}

/**
 * @brief 注册按键事件回调
 */
esp_err_t button_driver_register_callback(
    button_driver_handle_t handle,
    button_driver_event_t event,
    button_driver_callback_t callback,
    void *user_data
)
{
    if (handle == NULL || callback == NULL) {
        ESP_LOGE(TAG, "参数无效");
        return ESP_ERR_INVALID_ARG;
    }

    // 直接调用iot_button的回调注册接口
    esp_err_t ret = iot_button_register_cb(handle, event, NULL, callback, user_data);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "事件回调注册失败，event=%d", event);
        return ret;
    }

    ESP_LOGD(TAG, "事件回调注册成功，event=%d", event);
    return ESP_OK;
}

/**
 * @brief 获取按键按下时长
 */
uint32_t button_driver_get_press_time(button_driver_handle_t handle)
{
    if (handle == NULL) {
        return 0;
    }
    return iot_button_get_ticks_time(handle);
}

/**
 * @brief 删除按键设备
 */
esp_err_t button_driver_delete(button_driver_handle_t handle)
{
    if (handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t ret = iot_button_delete(handle);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "按键设备已删除");
    } else {
        ESP_LOGE(TAG, "按键删除失败");
    }

    return ret;
}
