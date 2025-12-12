/**
 * @file ac_service.c
 * @brief 智能空调控制服务实现
 */

#include "ac_service.h"
#include "rgb_pwm_driver.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <math.h>

static const char *TAG = "AC_SERVICE";

// 舒适区配置
static ac_service_config_t s_config;
static ac_status_t s_status = {
    .mode = AC_MODE_OFF,
    .intensity = 0,
    .target_temp = 24.0f,
    .auto_mode = false  // 修改为false，默认手动模式
};
static bool s_initialized = false;
static float s_current_temp = 25.0f;  // 当前温度传感器值
static bool s_led_enabled = true;     // 智能灯开关（默认开启）

// PWM呼吸灯动画控制
static TaskHandle_t s_animation_task = NULL;
static bool s_animation_running = false;
static uint8_t s_current_brightness = 0;
static bool s_brightness_increasing = true;

// PWM参数（根据温度差动态调整）
static uint8_t s_breathing_min = 30;   // 最小亮度 30%
static uint8_t s_breathing_max = 100;  // 最大亮度 100%
static uint8_t s_breathing_step = 2;   // 每次亮度变化步长
static uint16_t s_breathing_delay_ms = 30;  // 每步延迟（毫秒）

/**
 * @brief 根据温度差计算呼吸灯参数（高对比度版本）
 *
 * 温度差分级：
 * - 0-4°C：  10%-70%，慢速（step=2, delay=40ms）
 * - 4-8°C：  10%-85%，中速（step=3, delay=25ms）
 * - 8-12°C： 10%-100%，快速（step=4, delay=15ms）
 * - >12°C：  5%-100%，极快（step=5, delay=10ms）
 */
static void calculate_breathing_params(float temp_diff)
{
    if (temp_diff < 4.0f) {
        // 等级1：温度差小，缓慢呼吸，高对比度
        s_breathing_min = 10;
        s_breathing_max = 70;
        s_breathing_step = 2;
        s_breathing_delay_ms = 40;
        ESP_LOGI(TAG, "呼吸灯参数：等级1（温度差%.1f°C）- 10-70%%, 慢速", temp_diff);
    } else if (temp_diff < 8.0f) {
        // 等级2：温度差中等，中速呼吸，高对比度
        s_breathing_min = 10;
        s_breathing_max = 85;
        s_breathing_step = 3;
        s_breathing_delay_ms = 25;
        ESP_LOGI(TAG, "呼吸灯参数：等级2（温度差%.1f°C）- 10-85%%, 中速", temp_diff);
    } else if (temp_diff < 12.0f) {
        // 等级3：温度差大，快速呼吸，最大对比度
        s_breathing_min = 10;
        s_breathing_max = 100;
        s_breathing_step = 4;
        s_breathing_delay_ms = 15;
        ESP_LOGI(TAG, "呼吸灯参数：等级3（温度差%.1f°C）- 10-100%%, 快速", temp_diff);
    } else {
        // 等级4：温度差极大，极快呼吸，极致对比度
        s_breathing_min = 5;
        s_breathing_max = 100;
        s_breathing_step = 5;
        s_breathing_delay_ms = 10;
        ESP_LOGI(TAG, "呼吸灯参数：等级4（温度差%.1f°C）- 5-100%%, 极快", temp_diff);
    }

    // 重置当前亮度到新的最小值
    s_current_brightness = s_breathing_min;
    s_brightness_increasing = true;
}

/**
 * @brief PWM呼吸灯动画任务
 */
static void breathing_animation_task(void *arg)
{
    ESP_LOGI(TAG, "🔵🔴 呼吸灯动画任务启动（栈大小：3072字节）");

    ac_mode_t last_mode = s_status.mode;  // 记录上次的模式
    uint32_t loop_count = 0;

    while (s_animation_running) {
        loop_count++;

        // 检测模式是否被意外改变
        if (s_status.mode != last_mode) {
            ESP_LOGW(TAG, "⚠️ 呼吸灯任务检测到模式改变！从%d变为%d（循环%lu次）",
                     last_mode, s_status.mode, loop_count);
            last_mode = s_status.mode;
        }

        // 如果模式不是制冷或制热，停止任务
        if (s_status.mode != AC_MODE_COOLING && s_status.mode != AC_MODE_HEATING) {
            ESP_LOGW(TAG, "⚠️ 模式不是制冷/制热，呼吸灯任务退出（当前模式=%d）", s_status.mode);
            break;
        }

        rgb_color_t color = {0, 0, 0};

        // 根据当前模式设置颜色
        if (s_status.mode == AC_MODE_COOLING) {
            // 制冷：蓝色呼吸灯
            color.b = s_current_brightness;
        } else if (s_status.mode == AC_MODE_HEATING) {
            // 制热：红色呼吸灯
            color.r = s_current_brightness;
        }

        // 更新LED颜色
        rgb_pwm_set_color(&color);

        // 每100次循环输出一次状态（约3秒一次）
        if (loop_count % 100 == 0) {
            ESP_LOGI(TAG, "[LED] 呼吸灯运行中：亮度=%d%%, 模式=%d, 循环=%lu",
                     s_current_brightness, s_status.mode, loop_count);
        }

        // 计算下一个亮度值（使用动态参数）
        if (s_brightness_increasing) {
            s_current_brightness += s_breathing_step;
            if (s_current_brightness >= s_breathing_max) {
                s_current_brightness = s_breathing_max;
                s_brightness_increasing = false;
            }
        } else {
            if (s_current_brightness >= s_breathing_step) {
                s_current_brightness -= s_breathing_step;
            } else {
                s_current_brightness = s_breathing_min;
            }
            if (s_current_brightness <= s_breathing_min) {
                s_current_brightness = s_breathing_min;
                s_brightness_increasing = true;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(s_breathing_delay_ms));
    }

    ESP_LOGI(TAG, "🔚 呼吸灯动画任务结束（总循环%lu次）", loop_count);
    s_animation_running = false;  // 确保标志位正确
    s_animation_task = NULL;
    vTaskDelete(NULL);
}

/**
 * @brief 启动呼吸灯动画
 */
static void start_breathing_animation(void)
{
    // 计算温度差
    float temp_diff = fabsf(s_current_temp - s_status.target_temp);

    // 根据温度差设置呼吸灯参数
    calculate_breathing_params(temp_diff);

    // 如果已经在运行，不需要重新创建任务，只更新参数
    if (s_animation_running) {
        ESP_LOGI(TAG, "呼吸灯已在运行，仅更新参数（温度差%.1f°C）", temp_diff);
        return;
    }

    s_animation_running = true;

    // 创建呼吸灯任务（栈大小增加到3072防止溢出）
    BaseType_t ret = xTaskCreate(breathing_animation_task, "ac_breathing", 3072, NULL, 5, &s_animation_task);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "❌ 创建呼吸灯任务失败！");
        s_animation_running = false;
        return;
    }

    ESP_LOGI(TAG, "✅ 启动呼吸灯动画（当前温度%.1f°C, 目标温度%.1f°C, 温度差%.1f°C）",
             s_current_temp, s_status.target_temp, temp_diff);
}

/**
 * @brief 停止呼吸灯动画
 */
static void stop_breathing_animation(void)
{
    if (!s_animation_running) {
        ESP_LOGD(TAG, "呼吸灯未运行，无需停止");
        return;  // 没有运行
    }

    ESP_LOGI(TAG, "🛑 准备停止呼吸灯动画...");
    s_animation_running = false;

    // 等待任务结束
    int wait_count = 0;
    while (s_animation_task != NULL && wait_count < 50) {
        vTaskDelay(pdMS_TO_TICKS(10));
        wait_count++;
    }

    if (s_animation_task != NULL) {
        ESP_LOGW(TAG, "⚠️ 呼吸灯任务未能在500ms内结束");
    } else {
        ESP_LOGI(TAG, "✅ 呼吸灯动画已停止（等待%dms）", wait_count * 10);
    }
}

/**
 * @brief 计算温度偏离舒适区的程度（返回0-100）
 */
static uint8_t calculate_temp_deviation(float current_temp)
{
    float deviation = 0;

    if (current_temp < s_config.temp_min) {
        // 温度过低，需要制热
        deviation = s_config.temp_min - current_temp;
    } else if (current_temp > s_config.temp_max) {
        // 温度过高，需要制冷
        deviation = current_temp - s_config.temp_max;
    } else {
        // 在舒适区内
        return 0;
    }

    // 将偏离转换为强度百分比（最大偏离10度=100%）
    uint8_t intensity = (uint8_t)(deviation * 10.0f);
    if (intensity > 100) {
        intensity = 100;
    }

    return intensity;
}

/**
 * @brief 更新RGB灯颜色
 */
static esp_err_t update_rgb_led(void)
{
    rgb_color_t color = {0, 0, 0};

    ESP_LOGI(TAG, "[DEBUG] update_rgb_led - 模式=%d, 呼吸灯运行=%d, LED开关=%d",
             s_status.mode, s_animation_running, s_led_enabled);

    // 如果智能灯关闭，停止动画并关闭所有LED
    if (!s_led_enabled) {
        stop_breathing_animation();
        color.r = 0;
        color.g = 0;
        color.b = 0;
        rgb_pwm_set_color(&color);
        ESP_LOGI(TAG, "[LED] 智能灯已关闭（全黑）");
        return ESP_OK;
    }

    // 智能灯开启，根据模式显示颜色
    switch (s_status.mode) {
        case AC_MODE_OFF:
            // 停止动画，关闭所有LED
            stop_breathing_animation();
            color.r = 0;
            color.g = 0;
            color.b = 0;
            rgb_pwm_set_color(&color);
            ESP_LOGI(TAG, "[LED] 空调关闭");
            break;

        case AC_MODE_COMFORT:
            // 停止动画，显示绿色常亮
            stop_breathing_animation();
            color.r = 0;
            color.g = 100;  // 全亮度绿色
            color.b = 0;
            rgb_pwm_set_color(&color);
            ESP_LOGI(TAG, "🌿 环境舒适（绿色常亮）");
            break;

        case AC_MODE_HEATING:
            // 启动红色呼吸灯动画，模拟制热过程
            ESP_LOGI(TAG, "🔴 制热模式 - 红色呼吸灯");
            start_breathing_animation();
            break;

        case AC_MODE_COOLING:
            // 启动蓝色呼吸灯动画，模拟制冷过程
            ESP_LOGI(TAG, "🔵 制冷模式 - 蓝色呼吸灯");
            start_breathing_animation();
            break;
    }

    return ESP_OK;
}

esp_err_t ac_service_init(const ac_service_config_t *config)
{
    if (config == NULL) {
        ESP_LOGE(TAG, "配置参数为空");
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "========== 初始化智能空调服务 ==========");
    ESP_LOGI(TAG, "舒适温度区间: %.1f-%.1f°C", config->temp_min, config->temp_max);
    ESP_LOGI(TAG, "舒适湿度区间: %d-%d%%", config->humidity_min, config->humidity_max);
    ESP_LOGI(TAG, "烟雾阈值: %.2fV", config->smoke_threshold);

    // 保存配置
    s_config = *config;

    // 初始化RGB PWM驱动
    rgb_pwm_config_t rgb_config = {
        .gpio_r = config->gpio_r,
        .gpio_g = config->gpio_g,
        .gpio_b = config->gpio_b,
        .pwm_freq = 5000  // 5kHz PWM频率
    };

    esp_err_t ret = rgb_pwm_init(&rgb_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "RGB PWM初始化失败");
        return ret;
    }

    // 初始化为舒适模式（绿色常亮），手动控制
    s_status.mode = AC_MODE_COMFORT;
    s_status.intensity = 0;
    s_status.auto_mode = false;  // 默认手动模式，需用户主动开启空调

    // 初始状态：显示绿色常亮
    rgb_color_t init_color = { .r = 0, .g = 100, .b = 0 };
    rgb_pwm_set_color(&init_color);

    s_initialized = true;
    ESP_LOGI(TAG, "✅ 智能空调服务初始化完成");
    ESP_LOGI(TAG, "========================================");
    return ESP_OK;
}

esp_err_t ac_service_update(const sensor_data_t *sensor_data)
{
    if (!s_initialized) {
        ESP_LOGE(TAG, "服务未初始化");
        return ESP_ERR_INVALID_STATE;
    }

    if (sensor_data == NULL || !sensor_data->valid) {
        ESP_LOGW(TAG, "传感器数据无效");
        return ESP_ERR_INVALID_ARG;
    }

    float temp = sensor_data->temperature;
    int humidity = sensor_data->humidity;
    float smoke = sensor_data->smoke_voltage;

    // 保存旧温度，用于检测温度变化
    float old_temp = s_current_temp;
    s_current_temp = temp;  // 更新当前温度

    // 如果正在运行呼吸灯动画，且温度变化超过1度，重新计算参数
    if (s_animation_running && fabsf(temp - old_temp) > 1.0f) {
        float temp_diff = fabsf(s_current_temp - s_status.target_temp);
        calculate_breathing_params(temp_diff);
        ESP_LOGI(TAG, "温度变化超过1度，重新计算呼吸灯参数");
    }

    // 如果不是自动模式，只更新温度，不改变模式
    ESP_LOGI(TAG, "📊 update调用 - auto_mode=%d, 当前模式=%d, 当前温度=%.1f°C",
             s_status.auto_mode, s_status.mode, s_current_temp);
    if (!s_status.auto_mode) {
        return ESP_OK;
    }

    // 检查烟雾状态（优先级最高）
    if (smoke > s_config.smoke_threshold) {
        ESP_LOGW(TAG, "⚠️ 检测到烟雾，空调关闭");
        s_status.mode = AC_MODE_OFF;
        s_status.intensity = 0;
        return update_rgb_led();
    }

    // 检查温度是否在舒适区
    if (temp >= s_config.temp_min && temp <= s_config.temp_max &&
        humidity >= s_config.humidity_min && humidity <= s_config.humidity_max) {
        // 舒适状态
        s_status.mode = AC_MODE_COMFORT;
        s_status.intensity = 0;
    } else if (temp < s_config.temp_min) {
        // 温度过低，需要制热
        s_status.mode = AC_MODE_HEATING;
        s_status.intensity = calculate_temp_deviation(temp);
    } else if (temp > s_config.temp_max) {
        // 温度过高，需要制冷
        s_status.mode = AC_MODE_COOLING;
        s_status.intensity = calculate_temp_deviation(temp);
    } else {
        // 温度合适但湿度不合适，保持舒适模式
        s_status.mode = AC_MODE_COMFORT;
        s_status.intensity = 0;
    }

    return update_rgb_led();
}

esp_err_t ac_service_get_status(ac_status_t *status)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (status == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    *status = s_status;
    return ESP_OK;
}

esp_err_t ac_service_set_mode(ac_mode_t mode, uint8_t intensity)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (intensity > 100) {
        intensity = 100;
    }

    s_status.mode = mode;
    s_status.intensity = intensity;
    s_status.auto_mode = false;  // 手动设置后退出自动模式

    ESP_LOGI(TAG, "手动设置空调: 模式=%d, 强度=%d%%", mode, intensity);

    return update_rgb_led();
}

esp_err_t ac_service_set_auto(bool enable)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    s_status.auto_mode = enable;
    ESP_LOGI(TAG, "自动模式: %s", enable ? "启用" : "禁用");

    return ESP_OK;
}

esp_err_t ac_service_set_target_temp(float target_temp)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    s_status.target_temp = target_temp;
    ESP_LOGI(TAG, "设置目标温度: %.1f°C", target_temp);

    // 如果正在运行呼吸灯，重新计算参数
    if (s_animation_running) {
        float temp_diff = fabsf(s_current_temp - s_status.target_temp);
        calculate_breathing_params(temp_diff);
        ESP_LOGI(TAG, "重新计算呼吸灯参数（温度差%.1f°C）", temp_diff);
    }

    return ESP_OK;
}

esp_err_t ac_service_set_led_enabled(bool enabled)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    s_led_enabled = enabled;
    ESP_LOGI(TAG, "[LED] 智能灯: %s", enabled ? "开启" : "关闭");

    // 立即更新LED状态
    return update_rgb_led();
}

bool ac_service_is_led_enabled(void)
{
    return s_led_enabled;
}

esp_err_t ac_service_deinit(void)
{
    if (!s_initialized) {
        return ESP_OK;
    }

    // 停止呼吸灯动画
    stop_breathing_animation();

    rgb_pwm_deinit();
    s_initialized = false;

    ESP_LOGI(TAG, "空调服务已关闭");
    return ESP_OK;
}
