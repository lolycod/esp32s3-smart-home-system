/**
 * @file button_service.c
 * @brief 按键业务逻辑服务层实现
 */

#include "button_service.h"
#include "button_driver.h"
#include "esp_log.h"

static const char *TAG = "BUTTON_SERVICE";

// 按键句柄（服务层持有）
static button_driver_handle_t s_button_handle = NULL;

/**
 * @brief 单击事件处理函数
 *
 * 这里处理单击按键后的业务逻辑
 */
static void on_button_single_click(void *handle, void *user_data)
{
    ESP_LOGI(TAG, "✓ 单击事件触发");

    // TODO: 添加你的业务逻辑
    // 例如：
    // - 切换LED状态
    // - 发送消息到队列
    // - 通知其他服务
    // led_service_toggle();
}

/**
 * @brief 双击事件处理函数
 */
static void on_button_double_click(void *handle, void *user_data)
{
    ESP_LOGI(TAG, "✓✓ 双击事件触发");

    // TODO: 添加你的业务逻辑
    // 例如：
    // - 切换工作模式
    // - 触发配网流程
    // wifi_service_start_provisioning();
}

/**
 * @brief 长按开始事件处理函数
 */
static void on_button_long_press_start(void *handle, void *user_data)
{
    ESP_LOGI(TAG, "⏱ 长按开始");

    // TODO: 添加你的业务逻辑
    // 例如：
    // - 进入配置模式
    // - 重置设备
}

/**
 * @brief 长按保持事件处理函数
 */
static void on_button_long_press_hold(void *handle, void *user_data)
{
    uint32_t press_time = button_driver_get_press_time(handle);
    ESP_LOGI(TAG, "⏱ 长按保持中... 持续时间: %lu ms", press_time);

    // TODO: 添加你的业务逻辑
    // 例如：
    // - 超过5秒执行恢复出厂设置
    // if (press_time > 5000) {
    //     system_reset_to_factory();
    // }
}

/**
 * @brief 初始化按键服务
 */
esp_err_t button_service_init(const button_service_config_t *config)
{
    if (config == NULL) {
        ESP_LOGE(TAG, "配置参数为空");
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "========== 初始化按键服务 ==========");

    // 1. 配置按键驱动参数
    button_driver_config_t driver_cfg = {
        .gpio_num = config->gpio_num,
        .active_level = config->active_level,
        .long_press_time = 1000,    // 1秒触发长按
        .short_press_time = 180,    // 180ms去抖时间
    };

    // 2. 创建按键驱动
    esp_err_t ret = button_driver_create(&driver_cfg, &s_button_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "按键驱动创建失败");
        return ESP_FAIL;
    }

    // 3. 注册业务事件回调
    button_driver_register_callback(s_button_handle, BUTTON_SINGLE_CLICK, on_button_single_click, NULL);
    button_driver_register_callback(s_button_handle, BUTTON_DOUBLE_CLICK, on_button_double_click, NULL);
    button_driver_register_callback(s_button_handle, BUTTON_LONG_PRESS_START, on_button_long_press_start, NULL);
    button_driver_register_callback(s_button_handle, BUTTON_LONG_PRESS_HOLD, on_button_long_press_hold, NULL);

    ESP_LOGI(TAG, "✅ 按键服务初始化完成");
    ESP_LOGI(TAG, "📌 支持的按键事件：");
    ESP_LOGI(TAG, "  - 单击：快速按下并释放");
    ESP_LOGI(TAG, "  - 双击：连续快速点击两次");
    ESP_LOGI(TAG, "  - 长按：按住不放超过1秒");
    ESP_LOGI(TAG, "========================================");

    return ESP_OK;
}

/**
 * @brief 反初始化按键服务
 */
esp_err_t button_service_deinit(void)
{
    if (s_button_handle == NULL) {
        return ESP_OK;
    }

    esp_err_t ret = button_driver_delete(s_button_handle);
    if (ret == ESP_OK) {
        s_button_handle = NULL;
        ESP_LOGI(TAG, "按键服务已停止");
    }

    return ret;
}
