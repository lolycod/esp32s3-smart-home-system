/**
 * @file websocket_service.c
 * @brief WebSocket客户端服务实现（完整版）
 */

#include "websocket_service.h"
#include "esp_websocket_client.h"
#include "esp_event.h"
#include "esp_log.h"
#include "ac_service.h"
#include "hall_light_service.h"
#include "motor_service.h"
#include "servo_driver.h"
#include "cJSON.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static const char *TAG = "WS_SERVICE";
static esp_websocket_client_handle_t s_client = NULL;
static bool s_connected = false;

/**
 * @brief 处理接收到的命令
 */
static void handle_command(const char *command)
{
    ESP_LOGI(TAG, "处理命令: %s", command);

    // 空调控制命令
    if (strncmp(command, "AC_", 3) == 0) {
        if (strcmp(command, "AC_AUTO_ON") == 0) {
            // 开启智能控制
            ac_service_set_auto(true);
            ESP_LOGI(TAG, "[OK] 空调智能控制已启用");
        }
        else if (strcmp(command, "AC_AUTO_OFF") == 0) {
            // 关闭智能控制
            ac_service_set_auto(false);
            ESP_LOGI(TAG, "[OK] 空调智能控制已关闭");
        }
        else if (strcmp(command, "AC_OFF") == 0) {
            // 关闭空调（显示绿色）
            ac_service_set_mode(AC_MODE_COMFORT, 0);
            ESP_LOGI(TAG, "[OK] 空调已关闭");
        }
        else if (strncmp(command, "AC_COOLING_", 11) == 0) {
            // 制冷模式：AC_COOLING_24 （启动蓝色呼吸灯动画）
            int target_temp = atoi(command + 11);

            // 先设置目标温度
            ac_service_set_target_temp((float)target_temp);

            // 设置制冷模式（这会触发呼吸灯动画，根据温度差调整参数）
            ac_service_set_mode(AC_MODE_COOLING, 0);
            ESP_LOGI(TAG, "[OK] 空调制冷模式: 目标温度 %d C (蓝色呼吸灯)", target_temp);
        }
        else if (strncmp(command, "AC_HEATING_", 11) == 0) {
            // 制热模式：AC_HEATING_24 （启动红色呼吸灯动画）
            int target_temp = atoi(command + 11);

            // 先设置目标温度
            ac_service_set_target_temp((float)target_temp);

            // 设置制热模式（这会触发呼吸灯动画，根据温度差调整参数）
            ac_service_set_mode(AC_MODE_HEATING, 0);
            ESP_LOGI(TAG, "[OK] 空调制热模式: 目标温度 %d C (红色呼吸灯)", target_temp);
        }
    }
    // 风扇控制命令（电机）
    else if (strncmp(command, "MOTOR_", 6) == 0) {
        int speed = atoi(command + 6);
        if (speed >= 0 && speed <= 100) {
            if (speed == 0) {
                motor_service_stop(MOTOR_ID_1);
                ESP_LOGI(TAG, "[OK] 风扇已停止");
            } else {
                motor_service_forward(MOTOR_ID_1, (uint8_t)speed);
                ESP_LOGI(TAG, "[OK] 风扇转速设置为 %d%%", speed);
            }
        } else {
            ESP_LOGW(TAG, "[WARN] 风扇转速超出范围 (0-100): %d", speed);
        }
    }
    // 窗户控制命令（舵机）
    else if (strncmp(command, "SERVO_", 6) == 0) {
        int angle = atoi(command + 6);
        if (angle >= 0 && angle <= 180) {
            servo_set_angle((uint8_t)angle);
            ESP_LOGI(TAG, "[OK] 窗户角度设置为 %d 度", angle);
        } else {
            ESP_LOGW(TAG, "[WARN] 窗户角度超出范围 (0-180): %d", angle);
        }
    }
    // 指示灯控制命令
    else if (strncmp(command, "LED_", 4) == 0) {
        if (strcmp(command, "LED_ON") == 0) {
            ac_service_set_led_enabled(true);
            ESP_LOGI(TAG, "[OK] 指示灯已开启");
        }
        else if (strcmp(command, "LED_OFF") == 0) {
            ac_service_set_led_enabled(false);
            ESP_LOGI(TAG, "[OK] 指示灯已关闭");
        }
        else if (strcmp(command, "LED_GREEN") == 0) {
            // 设置绿灯（舒适模式）
            ac_service_set_mode(AC_MODE_COMFORT, 0);
            ESP_LOGI(TAG, "[OK] 指示灯设置为绿色 (正常状态)");
        }
        else if (strcmp(command, "LED_RED") == 0) {
            // 设置红灯（制热模式作为警戒状态）
            ac_service_set_mode(AC_MODE_HEATING, 100);
            ESP_LOGI(TAG, "[OK] 指示灯设置为红色 (警戒状态)");
        }
    }
    // 智能灯控制命令（复用空调LED，保留兼容性）
    else if (strncmp(command, "SMART_LIGHT_", 12) == 0) {
        if (strcmp(command, "SMART_LIGHT_ON") == 0) {
            ac_service_set_led_enabled(true);
            ESP_LOGI(TAG, "[OK] 智能灯已开启（显示当前模式颜色）");
        }
        else if (strcmp(command, "SMART_LIGHT_OFF") == 0) {
            ac_service_set_led_enabled(false);
            ESP_LOGI(TAG, "[OK] 智能灯已关闭（全黑）");
        }
    }
    else {
        ESP_LOGW(TAG, "未知命令: %s", command);
    }
}

static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;

    switch (event_id) {
    case WEBSOCKET_EVENT_CONNECTED:
        ESP_LOGI(TAG, "[OK] WebSocket连接成功");
        s_connected = true;
        break;
    case WEBSOCKET_EVENT_DISCONNECTED:
        ESP_LOGW(TAG, "[WARN] WebSocket断开连接");
        s_connected = false;
        break;
    case WEBSOCKET_EVENT_DATA:
        ESP_LOGI(TAG, "收到数据: %.*s", data->data_len, (char *)data->data_ptr);

        // 解析JSON消息
        if (data->data_len > 0 && data->data_ptr != NULL) {
            // 创建一个以null结尾的字符串
            char *msg = malloc(data->data_len + 1);
            if (msg != NULL) {
                memcpy(msg, data->data_ptr, data->data_len);
                msg[data->data_len] = '\0';

                // 解析JSON
                cJSON *json = cJSON_Parse(msg);
                if (json != NULL) {
                    // 获取data字段（命令字符串）
                    cJSON *data_field = cJSON_GetObjectItem(json, "data");
                    if (data_field != NULL && cJSON_IsString(data_field)) {
                        const char *command = data_field->valuestring;
                        handle_command(command);
                    }
                    cJSON_Delete(json);
                } else {
                    // 如果不是JSON，直接作为命令处理
                    handle_command(msg);
                }

                free(msg);
            }
        }
        break;
    case WEBSOCKET_EVENT_ERROR:
        ESP_LOGE(TAG, "[ERROR] WebSocket错误");
        s_connected = false;
        break;
    default:
        ESP_LOGI(TAG, "WebSocket事件: %ld", (long)event_id);
        break;
    }
}

esp_err_t websocket_service_init(const websocket_service_config_t *config)
{
    if (config == NULL || config->uri == NULL) {
        ESP_LOGE(TAG, "配置参数无效");
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "初始化WebSocket客户端");
    ESP_LOGI(TAG, "服务器URI: %s", config->uri);

    esp_websocket_client_config_t ws_cfg = {
        .uri = config->uri,
        .reconnect_timeout_ms = 5000,
        .network_timeout_ms = 10000,
        .ping_interval_sec = 30,  // 每30秒发送ping保活
        .disable_auto_reconnect = false,  // 启用自动重连
    };

    s_client = esp_websocket_client_init(&ws_cfg);
    if (s_client == NULL) {
        ESP_LOGE(TAG, "WebSocket客户端初始化失败");
        return ESP_FAIL;
    }

    esp_websocket_register_events(s_client, WEBSOCKET_EVENT_ANY, websocket_event_handler, NULL);

    esp_err_t ret = esp_websocket_client_start(s_client);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "[WARN] WebSocket客户端启动失败（WiFi可能未连接），将在后台自动重试");
        // 不返回错误，让它后台重连
    } else {
        ESP_LOGI(TAG, "[OK] WebSocket客户端已启动");
    }

    // [INFO] 异步连接模式：不阻塞等待，让WebSocket在后台自动连接
    // WebSocket客户端已配置自动重连（reconnect_timeout_ms = 5000）
    // WiFi连接成功后会自动建立WebSocket连接，不影响电机舵机立即启动
    ESP_LOGI(TAG, "[INFO] WebSocket服务已初始化（后台异步连接中...）");
    ESP_LOGI(TAG, "[INFO] WiFi连接成功后将自动建立WebSocket连接");
    return ESP_OK;
}

esp_err_t websocket_service_send_sensor_data(const sensor_data_t *data)
{
    if (!s_connected || s_client == NULL) {
        ESP_LOGD(TAG, "WebSocket未连接，跳过发送");
        return ESP_ERR_INVALID_STATE;
    }

    if (data == NULL || !data->valid) {
        return ESP_ERR_INVALID_ARG;
    }

    // 构建符合服务器要求的JSON数据（包含type, timestamp, data字段）
    char json[512];
    int len = snprintf(json, sizeof(json),
             "{\"type\":\"message\",\"timestamp\":%lu,\"data\":{\"temperature\":%.1f,\"humidity\":%d,\"smoke_voltage\":%.2f,\"smoke_detected\":%s,\"light_intensity\":%.1f,\"light_sufficient\":%s}}",
             data->timestamp_ms,
             data->temperature,
             data->humidity,
             data->smoke_voltage,
             data->smoke_detected ? "true" : "false",
             data->light_intensity,
             data->light_sufficient ? "true" : "false");

    // 发送数据
    int ret = esp_websocket_client_send_text(s_client, json, len, portMAX_DELAY);
    if (ret < 0) {
        ESP_LOGE(TAG, "发送数据失败: %d", ret);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "📤 发送数据: %s", json);
    return ESP_OK;
}

bool websocket_service_is_connected(void)
{
    return s_connected;
}

esp_err_t websocket_service_deinit(void)
{
    if (s_client) {
        esp_websocket_client_stop(s_client);
        esp_websocket_client_destroy(s_client);
        s_client = NULL;
        s_connected = false;
    }
    ESP_LOGI(TAG, "WebSocket服务已关闭");
    return ESP_OK;
}
