/**
 * @file motor_app.c
 * @brief Motor application layer implementation
 */

#include "motor_app.h"
#include "motor_service.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "MOTOR_APP";

static const motor_service_config_t s_motor_config = {
    .motor_count = 1,
    .motors = {
        {
            .gpio_in1 = 14,
            .gpio_in2 = 15,
            .gpio_pwm = 16,
            .pwm_freq = 5000,
            .ledc_timer = 2,
            .ledc_channel = 4,
        },
        {
            .gpio_in1 = 17,
            .gpio_in2 = 18,
            .gpio_pwm = 19,
            .pwm_freq = 5000,
            .ledc_timer = 2,
            .ledc_channel = 5,
        },
    }
};

esp_err_t motor_app_init(void)
{
    ESP_LOGI(TAG, "Motor App Init");
    esp_err_t ret = motor_service_init(&s_motor_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Motor service init failed");
        return ret;
    }
    ESP_LOGI(TAG, "Motor app init success");
    return ESP_OK;
}

esp_err_t motor_app_test_basic(void)
{
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "Basic Motor Test");
    motor_id_t motor_id = MOTOR_ID_1;

    ESP_LOGI(TAG, "[1/5] Motor forward 50%% speed");
    motor_service_forward(motor_id, 50);
    vTaskDelay(pdMS_TO_TICKS(2000));

    ESP_LOGI(TAG, "[2/5] Motor forward 100%% speed");
    motor_service_set_speed(motor_id, 100);
    vTaskDelay(pdMS_TO_TICKS(2000));

    ESP_LOGI(TAG, "[3/5] Motor stop");
    motor_service_stop(motor_id);
    vTaskDelay(pdMS_TO_TICKS(1000));

    ESP_LOGI(TAG, "[4/5] Motor backward 50%% speed");
    motor_service_backward(motor_id, 50);
    vTaskDelay(pdMS_TO_TICKS(2000));

    ESP_LOGI(TAG, "[5/5] Motor stop");
    motor_service_stop(motor_id);

    ESP_LOGI(TAG, "Basic motor test completed");
    return ESP_OK;
}

esp_err_t motor_app_test_speed_ramp(void)
{
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "Speed Ramp Test");
    motor_id_t motor_id = MOTOR_ID_1;

    ESP_LOGI(TAG, "[1/2] Acceleration test (0%% to 100%%)");
    motor_service_forward(motor_id, 0);

    for (uint8_t speed = 0; speed <= 100; speed += 10) {
        motor_service_set_speed(motor_id, speed);
        ESP_LOGI(TAG, "  Speed: %d%%", speed);
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    vTaskDelay(pdMS_TO_TICKS(1000));

    ESP_LOGI(TAG, "[2/2] Deceleration test (100%% to 0%%)");
    for (uint8_t speed = 100; speed > 0; speed -= 10) {
        motor_service_set_speed(motor_id, speed);
        ESP_LOGI(TAG, "  Speed: %d%%", speed);
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    motor_service_stop(motor_id);
    ESP_LOGI(TAG, "Speed ramp test completed");
    return ESP_OK;
}

esp_err_t motor_app_test_multi_motor(void)
{
    if (s_motor_config.motor_count < 2) {
        ESP_LOGI(TAG, "Multi-motor not configured, skipping test");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "Multi-Motor Test");

    ESP_LOGI(TAG, "[1/3] All motors forward 50%%");
    for (uint8_t i = 0; i < s_motor_config.motor_count; i++) {
        motor_service_forward((motor_id_t)i, 50);
    }
    vTaskDelay(pdMS_TO_TICKS(2000));

    ESP_LOGI(TAG, "[2/3] Different motors different speeds");
    for (uint8_t i = 0; i < s_motor_config.motor_count; i++) {
        uint8_t speed = 30 + (i * 20);
        motor_service_set_speed((motor_id_t)i, speed);
        ESP_LOGI(TAG, "  Motor %d: %d%%", i + 1, speed);
    }
    vTaskDelay(pdMS_TO_TICKS(2000));

    ESP_LOGI(TAG, "[3/3] Stop all motors");
    motor_service_stop_all();

    ESP_LOGI(TAG, "Multi-motor test completed");
    return ESP_OK;
}

esp_err_t motor_app_deinit(void)
{
    ESP_LOGI(TAG, "Motor app deinit");
    return motor_service_deinit();
}
