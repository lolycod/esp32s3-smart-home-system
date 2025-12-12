/**
 * @file main.c
 * @brief 系统启动器 - 简化版（仅DHT11温湿度监控）
 *
 * 这个文件的职责：
 * 1. 初始化NVS、系统外设
 * 2. 创建服务层和应用层所需的资源（如队列）
 * 3. 初始化各个服务和应用
 * 4. 启动FreeRTOS任务
 * 5. 让调度器接管
 *
 * @copyright Copyright (c) 2024
 */

#include <stdio.h>
#include "esp_log.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "nvs_flash.h"
#include "driver/adc.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_adc_cal.h"

// 服务层
#include "sensor_service.h"
#include "wifi_service.h"
#include "ac_service.h"
#include "hall_light_service.h"
#include "motor_service.h"

// 应用层
#include "temp_humidity_app.h"
#include "motor_app.h"

// 驱动层
#include "servo_driver.h"

static const char *TAG = "SYSTEM";

// 系统队列（服务层和应用层之间的数据通道）
static QueueHandle_t s_sensor_data_queue = NULL;

/**
 * @brief 电机持续正转任务（上电自动启动）
 */
static void motor_continuous_task(void *pvParameters)
{
    ESP_LOGI(TAG, "[MOTOR] 电机持续正转任务已启动");

    // 电机固定方向持续转动，速度80%
    motor_service_forward(MOTOR_ID_1, 80);
    ESP_LOGI(TAG, "[MOTOR] 电机已启动: 正转 80%% 速度 (持续运行)");

    // 任务保持运行，但不做任何操作（电机会一直转）
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(60000));  // 每分钟打印一次状态
        ESP_LOGI(TAG, "[MOTOR] 电机持续正转中...");
    }
}

// 舵机不需要持续运行任务，初始化后保持静止
// 如需控制舵机，使用WebSocket发送 SERVO_{角度} 命令

/**
 * @brief 初始化NVS（非易失性存储）
 */
static esp_err_t init_nvs(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "[OK] NVS初始化完成");
    return ESP_OK;
}

/**
 * @brief 初始化系统资源（队列、信号量等）
 */
static esp_err_t init_system_resources(void)
{
    ESP_LOGI(TAG, "========== 初始化系统资源 ==========");

    // 创建传感器数据队列（10个元素）
    s_sensor_data_queue = xQueueCreate(10, sizeof(sensor_data_t));
    if (s_sensor_data_queue == NULL) {
        ESP_LOGE(TAG, "[ERROR] 创建传感器数据队列失败");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "[OK] 系统资源初始化完成");
    ESP_LOGI(TAG, "   - 传感器数据队列: 已创建（容量10）");
    ESP_LOGI(TAG, "========================================");

    return ESP_OK;
}

/**
 * @brief 初始化所有服务层
 */
static esp_err_t init_services(void)
{
    esp_err_t ret;

    ESP_LOGI(TAG, "========== 初始化服务层 ==========");

    // [INFO] 优先初始化空调服务（显示绿灯，提升用户体验）
    ESP_LOGI(TAG, "[INFO] 优先初始化LED指示灯...");
    ac_service_config_t ac_cfg = {
        .temp_min = 20.0f,      // 舒适温度下限 20°C
        .temp_max = 26.0f,      // 舒适温度上限 26°C
        .humidity_min = 30,     // 舒适湿度下限 30%
        .humidity_max = 70,     // 舒适湿度上限 70%
        .smoke_threshold = 1.0f,  // 烟雾电压阈值 1.0V
        .gpio_r = 11,           // 外接RGB灯 红色通道
        .gpio_g = 12,           // 外接RGB灯 绿色通道
        .gpio_b = 13,           // 外接RGB灯 蓝色通道
    };
    ret = ac_service_init(&ac_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "[ERROR] 空调服务初始化失败");
        return ret;
    }
    ESP_LOGI(TAG, "[OK] LED指示灯已就绪（绿色）");

    // 初始化WiFi服务（异步非阻塞模式）
    ESP_LOGI(TAG, "[INFO] 初始化WiFi服务（异步模式，不阻塞电机启动）");
    wifi_service_config_t wifi_cfg = {
        .ssid = "123",
        .password = "123456abc",
        .max_retry = 10,
    };
    ret = wifi_service_init(&wifi_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "[ERROR] WiFi服务初始化失败");
        // 不返回，继续运行
    }

    // [INFO] 关键：WiFi连接不等待，让其在后台异步连接
    ret = wifi_service_connect();
    ESP_LOGI(TAG, "[INFO] WiFi后台连接中...（不阻塞，电机舵机继续启动）");

    // 初始化传感器服务
    sensor_service_config_t sensor_cfg = {
        .dht11_gpio = 4,
        .mq2_ttl_gpio = 5,
        .mq2_adc_channel = 5,
        .ldr_do_gpio = 8,         // 5516光敏电阻DO数字输出
        .ldr_adc_channel = 6,     // 5516光敏电阻AO模拟输出 (GPIO7=ADC1_CH6)
        .sample_interval_ms = 5000,
        .data_queue = s_sensor_data_queue,
        .websocket_uri = "ws://192.168.183.121:8080",  // 修改为你的PC IP地址
    };
    ret = sensor_service_init(&sensor_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "[ERROR] 传感器服务初始化失败");
        return ret;
    }

    // 初始化大厅灯服务（已禁用内置WS2812 RGB灯）
    // hall_light_config_t hall_cfg = {
    //     .gpio_pin = 48,         // ESP32S3内置RGB灯（WS2812）
    //     .brightness = 50,       // 默认亮度50%
    // };
    // ret = hall_light_init(&hall_cfg);
    // if (ret != ESP_OK) {
    //     ESP_LOGE(TAG, "[ERROR] 大厅灯服务初始化失败");
    //     return ret;
    // }
    ESP_LOGI(TAG, "[WARN] 内置WS2812 RGB灯已禁用");

    // 初始化舵机驱动（MG90S）
    servo_config_t servo_cfg = {
        .gpio_pin = 38,         // 舵机信号线GPIO
        .pwm_freq = 50,         // 50Hz标准频率
        .min_pulse = 500,       // 最小脉宽500μs (0度)
        .max_pulse = 2500,      // 最大脉宽2500μs (180度)
    };
    ret = servo_init(&servo_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "[ERROR] 舵机驱动初始化失败");
        return ret;
    }

    ESP_LOGI(TAG, "[OK] 所有服务初始化完成");
    ESP_LOGI(TAG, "========================================");

    return ESP_OK;
}

/**
 * @brief 初始化所有应用层
 */
static esp_err_t init_applications(void)
{
    esp_err_t ret;

    ESP_LOGI(TAG, "========== 初始化应用层 ==========");

    // 初始化温湿度应用
    temp_humidity_app_config_t app_cfg = {
        .sensor_data_queue = s_sensor_data_queue,
    };
    ret = temp_humidity_app_init(&app_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "[ERROR] 温湿度应用初始化失败");
        return ret;
    }

    // 初始化电机应用（TB6612直流电机驱动）
    ret = motor_app_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "[ERROR] 电机应用初始化失败");
        return ret;
    }

    ESP_LOGI(TAG, "[OK] 所有应用初始化完成");
    ESP_LOGI(TAG, "========================================");

    return ESP_OK;
}

/**
 * @brief 启动所有FreeRTOS任务
 */
static esp_err_t start_all_tasks(void)
{
    esp_err_t ret;

    ESP_LOGI(TAG, "========== 启动所有任务 ==========");

    // 1. 启动传感器服务任务
    ret = sensor_service_start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "[ERROR] 传感器服务启动失败");
        return ret;
    }

    // 2. 启动温湿度应用任务
    ret = temp_humidity_app_start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "[ERROR] 温湿度应用启动失败");
        return ret;
    }

    ESP_LOGI(TAG, "[OK] 所有任务已启动");
    ESP_LOGI(TAG, "========================================");

    return ESP_OK;
}

/**
 * @brief 打印系统架构信息
 */
static void print_system_architecture(void)
{
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "╔════════════════════════════════════════════════╗");
    ESP_LOGI(TAG, "║     ESP32 温湿度监控系统已启动                ║");
    ESP_LOGI(TAG, "╠════════════════════════════════════════════════╣");
    ESP_LOGI(TAG, "║  架构层次：                                    ║");
    ESP_LOGI(TAG, "║    [应用层] temp_humidity_app                  ║");
    ESP_LOGI(TAG, "║    [服务层] sensor_service                     ║");
    ESP_LOGI(TAG, "║    [驱动层] dht11_driver                       ║");
    ESP_LOGI(TAG, "║    [HAL层]  gpio_hal                           ║");
    ESP_LOGI(TAG, "╠════════════════════════════════════════════════╣");
    ESP_LOGI(TAG, "║  FreeRTOS任务：                                ║");
    ESP_LOGI(TAG, "║    - sensor_task (优先级5)                     ║");
    ESP_LOGI(TAG, "║    - temp_humi_app (优先级4)                   ║");
    ESP_LOGI(TAG, "╠════════════════════════════════════════════════╣");
    ESP_LOGI(TAG, "║  硬件配置：                                    ║");
    ESP_LOGI(TAG, "║    - DHT11温湿度传感器: GPIO4                  ║");
    ESP_LOGI(TAG, "║    - MQ-2烟雾传感器TTL: GPIO5                  ║");
    ESP_LOGI(TAG, "║    - MQ-2烟雾传感器ADC: GPIO6 (ADC1_CH5)       ║");
    ESP_LOGI(TAG, "╚════════════════════════════════════════════════╝");
    ESP_LOGI(TAG, "");
}

/* 这些临时测试函数已删除，功能已整合到主程序中 */

/**
 * @brief 简单ADC测试 - 直接测试GPIO6
 */
static void test_adc_gpio6_direct(void)
{
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "╔════════════════════════════════════════╗");
    ESP_LOGI(TAG, "║     GPIO6 ADC功能直接测试              ║");
    ESP_LOGI(TAG, "╚════════════════════════════════════════╝");
    ESP_LOGI(TAG, "");
    
    // 步骤1: 重置GPIO6
    ESP_LOGI(TAG, "[1/5] 重置GPIO6...");
    gpio_reset_pin(6);
    ESP_LOGI(TAG, "[OK] GPIO6已重置为默认状态");

    // 步骤2: 配置ADC1宽度
    ESP_LOGI(TAG, "[2/5] 配置ADC1宽度...");
    esp_err_t ret = adc1_config_width(ADC_WIDTH_BIT_12);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "[ERROR] ADC1宽度配置失败: %d", ret);
        return;
    }
    ESP_LOGI(TAG, "[OK] ADC1宽度: 12位 (0-4095)");

    // 步骤3: 配置ADC1_CH5(GPIO6)的衰减
    ESP_LOGI(TAG, "[3/5] 配置ADC1_CH5衰减...");
    ret = adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_11);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "[ERROR] ADC1通道衰减配置失败: %d", ret);
        return;
    }
    ESP_LOGI(TAG, "[OK] ADC1_CH5衰减: 11dB (测量范围0-3.3V)");

    // 步骤4: ADC校准
    ESP_LOGI(TAG, "[4/5] ADC校准...");
    esp_adc_cal_characteristics_t *adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    if (adc_chars == NULL) {
        ESP_LOGE(TAG, "[ERROR] 内存分配失败");
        return;
    }

    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(
        ADC_UNIT_1,
        ADC_ATTEN_DB_11,
        ADC_WIDTH_BIT_12,
        1100,
        adc_chars
    );

    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        ESP_LOGI(TAG, "[OK] 校准: eFuse Two Point");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        ESP_LOGI(TAG, "[OK] 校准: eFuse Vref");
    } else {
        ESP_LOGI(TAG, "[WARN] 校准: 默认Vref");
    }
    
    // 步骤5: 读取ADC 20次
    ESP_LOGI(TAG, "[5/5] 读取ADC (20次)...");
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "开始读取 GPIO6 (ADC1_CH5):");
    ESP_LOGI(TAG, "----------------------------------------");
    
    int zero_count = 0;
    int nonzero_count = 0;
    uint32_t sum = 0;
    int max_val = 0;
    int min_val = 4095;
    
    for (int i = 0; i < 20; i++) {
        // 直接读取ADC1_CH5
        int raw = adc1_get_raw(ADC1_CHANNEL_5);
        
        if (raw < 0) {
            ESP_LOGE(TAG, "[%2d] [ERROR] 读取失败: 错误码=%d", i+1, raw);
            continue;
        }
        
        // 转换为电压
        uint32_t voltage_mv = esp_adc_cal_raw_to_voltage(raw, adc_chars);
        float voltage_v = (float)voltage_mv / 1000.0f;
        
        // 统计
        if (raw == 0) {
            zero_count++;
        } else {
            nonzero_count++;
        }
        sum += raw;
        if (raw > max_val) max_val = raw;
        if (raw < min_val) min_val = raw;
        
        // 打印结果
        ESP_LOGI(TAG, "[%2d] RAW=%4d, 电压=%5lumV (%.3fV)", 
                 i+1, raw, voltage_mv, voltage_v);
        
        vTaskDelay(pdMS_TO_TICKS(100));  // 100ms间隔
    }
    
    // 统计结果
    ESP_LOGI(TAG, "----------------------------------------");
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "[INFO] 统计结果:");
    ESP_LOGI(TAG, "  - 读取次数: 20");
    ESP_LOGI(TAG, "  - 零值次数: %d", zero_count);
    ESP_LOGI(TAG, "  - 非零次数: %d", nonzero_count);
    ESP_LOGI(TAG, "  - 平均值: %lu", sum / 20);
    ESP_LOGI(TAG, "  - 最大值: %d", max_val);
    ESP_LOGI(TAG, "  - 最小值: %d", min_val);
    ESP_LOGI(TAG, "  - 波动: %d", max_val - min_val);
    ESP_LOGI(TAG, "");

    // 诊断
    if (zero_count == 20) {
        ESP_LOGE(TAG, "[CRITICAL] 严重问题: 所有读取都为0!");
        ESP_LOGE(TAG, "");
        ESP_LOGE(TAG, "这说明ADC硬件功能有问题,可能原因:");
        ESP_LOGE(TAG, "  1. GPIO6被eFuse锁定或禁用");
        ESP_LOGE(TAG, "  2. ESP32-S3芯片型号不支持此ADC通道");
        ESP_LOGE(TAG, "  3. 硬件损坏");
        ESP_LOGE(TAG, "  4. ESP-IDF版本不兼容");
        ESP_LOGE(TAG, "");
        ESP_LOGE(TAG, "建议:");
        ESP_LOGE(TAG, "  1. 尝试其他GPIO (GPIO7/GPIO8/GPIO9)");
        ESP_LOGE(TAG, "  2. 检查ESP-IDF版本");
        ESP_LOGE(TAG, "  3. 更换ESP32开发板");
    } else if (zero_count > 10) {
        ESP_LOGW(TAG, "[WARN] 大部分读取为0,ADC功能不稳定");
    } else if (nonzero_count == 20) {
        ESP_LOGI(TAG, "[OK] ADC功能完全正常!");
        ESP_LOGI(TAG, "");
        ESP_LOGI(TAG, "ADC硬件工作正常,问题可能在:");
        ESP_LOGI(TAG, "  1. MQ-2传感器未连接或未供电");
        ESP_LOGI(TAG, "  2. 接线错误");
        ESP_LOGI(TAG, "  3. MQ-2模块损坏");
    } else {
        ESP_LOGI(TAG, "[OK] ADC功能基本正常");
    }
    
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "╔════════════════════════════════════════╗");
    ESP_LOGI(TAG, "║     测试完成                           ║");
    ESP_LOGI(TAG, "╚════════════════════════════════════════╝");
    ESP_LOGI(TAG, "");
    
    free(adc_chars);
}

/* GPIO38和LEDC临时测试函数已删除，功能已整合到舵机驱动中 */

/**
 * @brief 应用程序主入口
 */
void app_main(void)
{
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "==========================================");
    ESP_LOGI(TAG, "       ESP32 系统启动中...               ");
    ESP_LOGI(TAG, "==========================================");
    ESP_LOGI(TAG, "");

    // 1. 初始化NVS
    init_nvs();

    // 2. 初始化系统资源
    if (init_system_resources() != ESP_OK) {
        ESP_LOGE(TAG, "系统资源初始化失败，系统停止");
        return;
    }

    // 3. 初始化服务层
    if (init_services() != ESP_OK) {
        ESP_LOGE(TAG, "服务层初始化失败，系统停止");
        return;
    }

    // 4. 初始化应用层
    if (init_applications() != ESP_OK) {
        ESP_LOGE(TAG, "应用层初始化失败，系统停止");
        return;
    }

    // 5. 启动所有FreeRTOS任务
    if (start_all_tasks() != ESP_OK) {
        ESP_LOGE(TAG, "任务启动失败，系统停止");
        return;
    }

    // 6. 打印系统架构信息
    print_system_architecture();

    ESP_LOGI(TAG, "[OK] 系统启动完成! FreeRTOS调度器已接管");
    ESP_LOGI(TAG, "");

    // 启动电机持续正转任务
    xTaskCreate(motor_continuous_task, "motor_continuous", 2048, NULL, 5, NULL);
    ESP_LOGI(TAG, "[OK] 电机持续正转任务已创建");

    // main任务完成初始化后进入空闲循环
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(60000));  // 每分钟打印一次状态
        ESP_LOGI(TAG, "[SYSTEM] 系统运行正常 | 空闲堆内存: %lu bytes", esp_get_free_heap_size());
    }
}
