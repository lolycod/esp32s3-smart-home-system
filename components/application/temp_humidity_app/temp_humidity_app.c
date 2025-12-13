/**
 * @file temp_humidity_app.c
 * @brief 温湿度应用层实现
 */

#include "temp_humidity_app.h"
#include "sensor_service.h"
#include "ac_service.h"
#include "esp_log.h"
#include "freertos/task.h"

static const char *TAG = "TEMP_HUMIDITY_APP";

// 应用状态
static bool s_is_running = false;
static TaskHandle_t s_task_handle = NULL;
static QueueHandle_t s_sensor_data_queue = NULL;

// 统计信息
static uint32_t s_total_samples = 0;
static float s_avg_temperature = 0.0f;
static uint8_t s_avg_humidity = 0;

/**
 * @brief 温湿度显示任务（现包含烟雾数据）
 */
static void temp_humidity_display_task(void *arg)
{
    ESP_LOGI(TAG, "📱 温湿度显示任务已启动");

    sensor_data_t data;

    while (s_is_running) {
        // 从队列接收传感器数据
        if (xQueueReceive(s_sensor_data_queue, &data, pdMS_TO_TICKS(1000)) == pdTRUE) {
            if (data.valid) {
                s_total_samples++;

                // 计算平均值（简单累加平均）
                s_avg_temperature = ((s_avg_temperature * (s_total_samples - 1)) + data.temperature) / s_total_samples;
                s_avg_humidity = ((s_avg_humidity * (s_total_samples - 1)) + data.humidity) / s_total_samples;

                // 更新空调服务状态（仅在自动模式下调整）
                // 手动模式下也需要调用以更新温度传感器值
                ac_service_update(&data);

                // 显示当前数据
                ESP_LOGI(TAG, "╔════════════════════════════════════════╗");
                ESP_LOGI(TAG, "║   🌡️ 多传感器环境监控系统  💧 ⚠️       ║");
                ESP_LOGI(TAG, "╠════════════════════════════════════════╣");
                ESP_LOGI(TAG, "║  当前温度: %.1f°C                     ║", data.temperature);
                ESP_LOGI(TAG, "║  当前湿度: %d%%                        ║", data.humidity);

                // 烟雾传感器数据和浓度等级显示
                if (data.smoke_detected) {
                    ESP_LOGW(TAG, "║ 🔥 警报: 检测到烟雾！                   ║");
                } else {
                    ESP_LOGI(TAG, "║ 🌫️ 烟雾状态: 正常                       ║");
                }

                // 根据浓度百分比分级显示
                const char* level_text = "";
                const char* level_emoji = "";
                if (data.smoke_voltage < 0.3f) {
                    level_text = "空气清新";
                    level_emoji = "✨";
                } else if (data.smoke_voltage < 1.0f) {
                    level_text = "轻度污染";
                    level_emoji = "🟡";
                } else if (data.smoke_voltage < 2.0f) {
                    level_text = "中度污染";
                    level_emoji = "🟠";
                } else {
                    level_text = "重度污染";
                    level_emoji = "🔴";
                }

                float concentration_pct = (data.smoke_voltage / 3.3f) * 100.0f;
                ESP_LOGI(TAG, "║ %s %s: %.1f%% (%.2fV)              ║",
                          level_emoji, level_text, concentration_pct, data.smoke_voltage);
                ESP_LOGI(TAG, "║  时间戳  : %lu ms                  ║", data.timestamp_ms);
                ESP_LOGI(TAG, "╠════════════════════════════════════════╣");
                ESP_LOGI(TAG, "║  平均温度: %.1f°C                     ║", s_avg_temperature);
                ESP_LOGI(TAG, "║  平均湿度: %d%%                        ║", s_avg_humidity);
                ESP_LOGI(TAG, "║  采样次数: %lu                       ║", s_total_samples);
                ESP_LOGI(TAG, "╚════════════════════════════════════════╝");

                // 简单的告警逻辑示例
                if (data.temperature > 30.0f) {
                    ESP_LOGW(TAG, "⚠️ 温度过高告警！当前温度: %.1f°C", data.temperature);
                }
                if (data.humidity > 80) {
                    ESP_LOGW(TAG, "⚠️ 湿度过高告警！当前湿度: %d%%", data.humidity);
                }

                // 烟雾浓度告警逻辑
                if (concentration_pct > 60.0f) {
                    ESP_LOGE(TAG, "🔥 严重烟雾告警！浓度: %.1f%% (%.2fV) - 立即采取行动！",
                             concentration_pct, data.smoke_voltage);
                } else if (concentration_pct > 30.0f) {
                    ESP_LOGW(TAG, "⚠️ 烟雾浓度告警！浓度: %.1f%% (%.2fV) - 建议通风",
                             concentration_pct, data.smoke_voltage);
                }
            } else {
                ESP_LOGW(TAG, "⚠️ 收到无效的传感器数据");
            }
        } else {
            // 队列超时，继续等待
            ESP_LOGD(TAG, "等待传感器数据...");
        }
    }

    ESP_LOGI(TAG, "温湿度显示任务已结束");
    s_task_handle = NULL;
    vTaskDelete(NULL);
}

/**
 * @brief 初始化温湿度应用
 */
esp_err_t temp_humidity_app_init(const temp_humidity_app_config_t *config)
{
    if (config == NULL) {
        ESP_LOGE(TAG, "配置参数为空");
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "========== 初始化温湿度应用 ==========");

    s_sensor_data_queue = config->sensor_data_queue;

    if (s_sensor_data_queue == NULL) {
        ESP_LOGE(TAG, "❌ 传感器数据队列未配置");
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "✅ 温湿度应用初始化完成");
    ESP_LOGI(TAG, "========================================");

    return ESP_OK;
}

/**
 * @brief 启动温湿度应用任务
 */
esp_err_t temp_humidity_app_start(void)
{
    if (s_is_running) {
        ESP_LOGW(TAG, "温湿度应用已在运行中");
        return ESP_OK;
    }

    s_is_running = true;

    // 创建显示任务
    BaseType_t ret = xTaskCreate(
        temp_humidity_display_task,
        "temp_humi_app",
        4096,
        NULL,
        4,  // 优先级（低于传感器任务）
        &s_task_handle
    );

    if (ret != pdPASS) {
        ESP_LOGE(TAG, "❌ 创建温湿度显示任务失败");
        s_is_running = false;
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "🚀 温湿度应用已启动");
    return ESP_OK;
}

/**
 * @brief 停止温湿度应用任务
 */
esp_err_t temp_humidity_app_stop(void)
{
    if (!s_is_running) {
        return ESP_OK;
    }

    s_is_running = false;

    // 等待任务结束
    if (s_task_handle != NULL) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    ESP_LOGI(TAG, "温湿度应用已停止");
    return ESP_OK;
}

/**
 * @brief 反初始化温湿度应用
 */
esp_err_t temp_humidity_app_deinit(void)
{
    temp_humidity_app_stop();

    ESP_LOGI(TAG, "温湿度应用已关闭");
    return ESP_OK;
}
