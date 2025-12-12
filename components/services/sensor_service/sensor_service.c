/**
 * @file sensor_service.c
 * @brief 传感器业务逻辑服务层实现
 */

#include "sensor_service.h"
#include "dht11_driver.h"
#include "mq2_driver.h"
#include "ldr_driver.h"
#include "websocket_service.h"
#include "esp_log.h"
#include "freertos/task.h"
#include "esp_timer.h"

static const char *TAG = "SENSOR_SERVICE";

// 服务状态
static bool s_is_running = false;
static TaskHandle_t s_task_handle = NULL;
static uint32_t s_sample_interval_ms = 5000;
static QueueHandle_t s_data_queue = NULL;
static bool s_websocket_enabled = false;
static sensor_data_t s_last_sensor_data = {0};  // 保存最后一次传感器数据
static bool s_last_ws_connected = false;  // 记录上次WebSocket连接状态

/**
 * @brief 传感器数据采集任务
 */
static void sensor_task(void *arg)
{
    ESP_LOGI(TAG, "🌡️ 传感器采集任务已启动");

    TickType_t last_wake_time = xTaskGetTickCount();
    bool first_run = true;  // 标记首次运行

    while (s_is_running) {
        sensor_data_t data = {0};
        dht11_data_t dht11_data;
        mq2_data_t mq2_data;
        ldr_data_t ldr_data;

        // 读取DHT11数据
        esp_err_t ret = dht11_driver_read(&dht11_data);
        if (ret == ESP_OK) {
            data.temperature = dht11_data.temperature;
            data.humidity = dht11_data.humidity;
            data.timestamp_ms = (uint32_t)(esp_timer_get_time() / 1000);
            data.valid = true;
        } else {
            ESP_LOGW(TAG, "❌ DHT11读取失败");
            data.valid = false;
        }

        // 读取MQ-2数据
        ret = mq2_driver_read(&mq2_data);
        if (ret == ESP_OK) {
            data.smoke_detected = mq2_data.smoke_detected;
            data.smoke_voltage = mq2_data.voltage;
        }

        // 读取5516光敏电阻数据
        ret = ldr_driver_read(&ldr_data);
        if (ret == ESP_OK) {
            data.light_intensity = ldr_data.light_intensity;
            data.light_sufficient = ldr_data.light_sufficient;
        } else {
            data.light_intensity = 0.0f;
            data.light_sufficient = false;
        }

        ESP_LOGI(TAG, "📊 温度: %.1f°C | 湿度: %d%% | 烟雾: %.2fV | 光照: %.1f%%",
                 data.temperature, data.humidity, data.smoke_voltage, data.light_intensity);

        // 保存最后一次传感器数据
        s_last_sensor_data = data;

        // 检测WebSocket连接状态变化
        bool current_ws_connected = s_websocket_enabled && websocket_service_is_connected();
        bool ws_just_connected = current_ws_connected && !s_last_ws_connected;
        s_last_ws_connected = current_ws_connected;

        // 如果WebSocket刚连接上，立即发送一次数据
        if (ws_just_connected && data.valid) {
            ESP_LOGI(TAG, "🔔 WebSocket刚连接，立即发送当前数据");
            websocket_service_send_sensor_data(&data);
        }
        // 正常周期发送
        else if (s_websocket_enabled && data.valid) {
            websocket_service_send_sensor_data(&data);
        }

        // 发送到队列
        if (s_data_queue != NULL) {
            if (xQueueSend(s_data_queue, &data, pdMS_TO_TICKS(100)) != pdTRUE) {
                ESP_LOGW(TAG, "⚠️ 数据队列已满，丢弃本次数据");
            }
        }

        // 周期性延时（首次运行立即采样，之后按周期）
        if (first_run) {
            first_run = false;
            ESP_LOGI(TAG, "✅ 首次数据采样完成，立即可用");
            last_wake_time = xTaskGetTickCount();  // 重置基准时间
        }
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(s_sample_interval_ms));
    }

    ESP_LOGI(TAG, "传感器采集任务已结束");
    s_task_handle = NULL;
    vTaskDelete(NULL);
}

/**
 * @brief 初始化传感器服务
 */
esp_err_t sensor_service_init(const sensor_service_config_t *config)
{
    if (config == NULL) {
        ESP_LOGE(TAG, "配置参数为空");
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "========== 初始化传感器服务 ==========");

    s_sample_interval_ms = config->sample_interval_ms;
    s_data_queue = config->data_queue;

    // [DEBUG] Test GPIO4 basic functionality first
    ESP_LOGI(TAG, "[TEST] Starting GPIO4 basic test...");
    esp_err_t test_ret = test_gpio4_basic();
    if (test_ret != ESP_OK) {
        ESP_LOGE(TAG, "[TEST] GPIO4 basic test FAILED, cannot continue DHT11 init");
        return test_ret;
    }
    ESP_LOGI(TAG, "[TEST] GPIO4 basic test PASSED");

    // Initialize DHT11 driver
    dht11_driver_config_t dht11_cfg = {
        .gpio_num = config->dht11_gpio,
    };

    esp_err_t ret = dht11_driver_init(&dht11_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "DHT11 driver init failed");
        return ret;
    }

    // Initialize MQ-2 driver
    mq2_driver_config_t mq2_cfg = {
        .ttl_gpio = config->mq2_ttl_gpio,
        .adc_channel = config->mq2_adc_channel,
        .sample_interval_ms = config->sample_interval_ms,
    };

    ret = mq2_driver_init(&mq2_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "MQ-2 driver init failed");
        return ret;
    }

    // Initialize 5516光敏电阻 driver
    ldr_driver_config_t ldr_cfg = {
        .do_gpio = config->ldr_do_gpio,
        .adc_channel = config->ldr_adc_channel,
        .sample_interval_ms = config->sample_interval_ms,
    };

    ret = ldr_driver_init(&ldr_cfg);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "⚠️ LDR driver init failed, continuing without light sensor");
        // 不返回错误，继续运行（没有光敏电阻也能工作）
    } else {
        ESP_LOGI(TAG, "✅ LDR driver initialized successfully");
    }

    // 初始化WebSocket服务（如果配置了）
    if (config->websocket_uri != NULL) {
        websocket_service_config_t ws_cfg = {
            .uri = config->websocket_uri,
            .send_interval_ms = config->sample_interval_ms,
        };
        ret = websocket_service_init(&ws_cfg);
        if (ret == ESP_OK) {
            s_websocket_enabled = true;
            ESP_LOGI(TAG, "✅ WebSocket服务已启用");
        } else {
            ESP_LOGW(TAG, "⚠️ WebSocket服务初始化失败，继续运行");
        }
    }

    ESP_LOGI(TAG, "✅ 传感器服务初始化完成");
    ESP_LOGI(TAG, "   - DHT11 GPIO: %d", config->dht11_gpio);
    ESP_LOGI(TAG, "   - MQ-2 TTL GPIO: %d", config->mq2_ttl_gpio);
    ESP_LOGI(TAG, "   - MQ-2 ADC通道: %d", config->mq2_adc_channel);
    ESP_LOGI(TAG, "   - LDR DO GPIO: %d", config->ldr_do_gpio);
    ESP_LOGI(TAG, "   - LDR ADC通道: %d", config->ldr_adc_channel);
    ESP_LOGI(TAG, "   - 采样间隔: %lu ms", s_sample_interval_ms);
    ESP_LOGI(TAG, "   - 数据队列: %s", s_data_queue ? "启用" : "禁用");
    ESP_LOGI(TAG, "   - WebSocket: %s", s_websocket_enabled ? "启用" : "禁用");
    ESP_LOGI(TAG, "========================================");

    return ESP_OK;
}

/**
 * @brief 启动传感器数据采集
 */
esp_err_t sensor_service_start(void)
{
    if (s_is_running) {
        ESP_LOGW(TAG, "传感器服务已在运行中");
        return ESP_OK;
    }

    s_is_running = true;

    // 创建采集任务
    BaseType_t ret = xTaskCreate(
        sensor_task,
        "sensor_task",
        4096,
        NULL,
        5,  // 优先级
        &s_task_handle
    );

    if (ret != pdPASS) {
        ESP_LOGE(TAG, "❌ 创建传感器任务失败");
        s_is_running = false;
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "🚀 传感器数据采集已启动");
    return ESP_OK;
}

/**
 * @brief 停止传感器数据采集
 */
esp_err_t sensor_service_stop(void)
{
    if (!s_is_running) {
        return ESP_OK;
    }

    s_is_running = false;

    // 等待任务结束
    if (s_task_handle != NULL) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    ESP_LOGI(TAG, "传感器数据采集已停止");
    return ESP_OK;
}

/**
 * @brief 手动读取一次传感器数据
 */
esp_err_t sensor_service_read_once(sensor_data_t *data)
{
    if (data == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    // 初始化字段
    data->temperature = 0.0f;
    data->humidity = 0;
    data->smoke_detected = false;
    data->smoke_voltage = 0.0f;
    data->timestamp_ms = (uint32_t)(esp_timer_get_time() / 1000);
    data->valid = true;

    // 读取DHT11数据
    dht11_data_t dht11_data;
    esp_err_t ret = dht11_driver_read(&dht11_data);

    if (ret == ESP_OK) {
        data->temperature = dht11_data.temperature;
        data->humidity = dht11_data.humidity;
    } else {
        ESP_LOGW(TAG, "DHT11读数失败");
    }

    // 读取MQ-2数据
    mq2_data_t mq2_data;
    ret = mq2_driver_read(&mq2_data);

    if (ret == ESP_OK) {
        data->smoke_detected = mq2_data.smoke_detected;
        data->smoke_voltage = mq2_data.voltage;
    } else {
        ESP_LOGW(TAG, "MQ-2读数失败");
    }

    return ESP_OK;
}

/**
 * @brief 反初始化传感器服务
 */
esp_err_t sensor_service_deinit(void)
{
    sensor_service_stop();
    dht11_driver_deinit();
    mq2_driver_deinit();

    ESP_LOGI(TAG, "传感器服务已关闭");
    return ESP_OK;
}
