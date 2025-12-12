/**
 * @file ldr_driver.c
 * @brief 5516光敏电阻模块驱动实现
 */

#include "ldr_driver.h"

#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "LDR_DRIVER";

// 配置参数
static int s_do_gpio = -1;
static int s_adc_channel = -1;
static adc1_channel_t s_adc1_channel_enum = ADC1_CHANNEL_0;
static bool s_initialized = false;

// ADC配置
static esp_adc_cal_characteristics_t *adc_chars;
static const adc_bits_width_t adc_width = ADC_WIDTH_BIT_12;
static const adc_atten_t adc_atten = ADC_ATTEN_DB_11;  // 11dB衰减，支持0-3.3V

/**
 * @brief 初始化GPIO（DO数字输出）
 */
static esp_err_t init_do_gpio(int gpio_num)
{
    if (gpio_num < 0) {
        ESP_LOGW(TAG, "⚠️ DO GPIO未配置，跳过初始化");
        return ESP_OK;
    }

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << gpio_num),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,   // 启用上拉
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    esp_err_t ret = gpio_config(&io_conf);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "✅ DO GPIO初始化成功: GPIO%d", gpio_num);
    } else {
        ESP_LOGE(TAG, "❌ DO GPIO初始化失败: GPIO%d", gpio_num);
    }

    return ret;
}

/**
 * @brief 初始化ADC（AO模拟输出）
 *
 * ESP32-S3 ADC1通道映射:
 * - ADC1_CH6 → GPIO7 (推荐用于LDR AO)
 * - ADC1_CH7 → GPIO8
 */
static esp_err_t init_adc(int channel)
{
    if (channel < 0) {
        ESP_LOGW(TAG, "⚠️ ADC通道未配置，跳过初始化");
        return ESP_OK;
    }

    // 验证ADC通道范围
    if (channel < 0 || channel > 9) {
        ESP_LOGE(TAG, "❌ 无效的ADC通道: %d (有效范围: 0-9)", channel);
        return ESP_ERR_INVALID_ARG;
    }

    // ESP32-S3 ADC1通道映射
    adc1_channel_t adc_channel;
    int gpio_num = -1;

    switch (channel) {
        case 0: adc_channel = ADC1_CHANNEL_0; gpio_num = 1; break;
        case 1: adc_channel = ADC1_CHANNEL_1; gpio_num = 2; break;
        case 2: adc_channel = ADC1_CHANNEL_2; gpio_num = 3; break;
        case 3: adc_channel = ADC1_CHANNEL_3; gpio_num = 4; break;
        case 4: adc_channel = ADC1_CHANNEL_4; gpio_num = 5; break;
        case 5: adc_channel = ADC1_CHANNEL_5; gpio_num = 6; break;
        case 6: adc_channel = ADC1_CHANNEL_6; gpio_num = 7; break;  // ⭐推荐用于LDR
        case 7: adc_channel = ADC1_CHANNEL_7; gpio_num = 8; break;
        case 8: adc_channel = ADC1_CHANNEL_8; gpio_num = 9; break;
        case 9: adc_channel = ADC1_CHANNEL_9; gpio_num = 10; break;
        default:
            ESP_LOGE(TAG, "❌ 不支持的ADC通道: %d", channel);
            return ESP_ERR_INVALID_ARG;
    }

    // 保存枚举值
    s_adc1_channel_enum = adc_channel;

    // 配置ADC宽度
    esp_err_t ret = adc1_config_width(adc_width);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "❌ ADC宽度配置失败");
        return ret;
    }

    // 配置ADC衰减
    ret = adc1_config_channel_atten(adc_channel, adc_atten);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "❌ ADC衰减配置失败");
        return ret;
    }

    // 校准ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    if (adc_chars == NULL) {
        ESP_LOGE(TAG, "❌ 内存分配失败");
        return ESP_ERR_NO_MEM;
    }

    esp_adc_cal_characterize(ADC_UNIT_1, adc_atten, adc_width, 0, adc_chars);

    ESP_LOGI(TAG, "✅ ADC初始化成功: ADC1_CH%d (GPIO%d)", channel, gpio_num);

    return ESP_OK;
}

esp_err_t ldr_driver_init(const ldr_driver_config_t *config)
{
    if (config == NULL) {
        ESP_LOGE(TAG, "❌ 配置参数为空");
        return ESP_ERR_INVALID_ARG;
    }

    if (s_initialized) {
        ESP_LOGW(TAG, "⚠️ 驱动已初始化");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "========== 初始化5516光敏电阻驱动 ==========");

    // 保存配置
    s_do_gpio = config->do_gpio;
    s_adc_channel = config->adc_channel;

    // 初始化DO GPIO
    esp_err_t ret = init_do_gpio(s_do_gpio);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "❌ DO GPIO初始化失败");
        return ret;
    }

    // 初始化ADC
    ret = init_adc(s_adc_channel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "❌ ADC初始化失败");
        return ret;
    }

    s_initialized = true;

    ESP_LOGI(TAG, "✅ 5516光敏电阻驱动初始化完成");
    ESP_LOGI(TAG, "   - DO GPIO: %d", s_do_gpio);
    ESP_LOGI(TAG, "   - ADC通道: %d", s_adc_channel);
    ESP_LOGI(TAG, "========================================");

    return ESP_OK;
}

/**
 * @brief 根据光照强度百分比获取光照等级
 */
static light_level_t get_light_level(float intensity)
{
    if (intensity <= 20.0f) {
        return LIGHT_LEVEL_DARK;
    } else if (intensity <= 40.0f) {
        return LIGHT_LEVEL_DIM;
    } else if (intensity <= 60.0f) {
        return LIGHT_LEVEL_MODERATE;
    } else if (intensity <= 80.0f) {
        return LIGHT_LEVEL_BRIGHT;
    } else {
        return LIGHT_LEVEL_VERY_BRIGHT;
    }
}

esp_err_t ldr_driver_read(ldr_data_t *data)
{
    if (!s_initialized) {
        ESP_LOGE(TAG, "❌ 驱动未初始化");
        return ESP_ERR_INVALID_STATE;
    }

    if (data == NULL) {
        ESP_LOGE(TAG, "❌ 数据指针为空");
        return ESP_ERR_INVALID_ARG;
    }

    // 读取DO数字输出（高电平=光照充足，低电平=光照不足）
    if (s_do_gpio >= 0) {
        int do_level = gpio_get_level(s_do_gpio);
        data->light_sufficient = (do_level == 1);  // 高电平表示光照充足
    } else {
        data->light_sufficient = true;  // 默认充足
    }

    // 读取AO模拟输出
    if (s_adc_channel >= 0) {
        // 多次采样求平均值，提高稳定性
        uint32_t adc_sum = 0;
        const int samples = 16;

        for (int i = 0; i < samples; i++) {
            adc_sum += adc1_get_raw(s_adc1_channel_enum);
            vTaskDelay(pdMS_TO_TICKS(2));  // 短暂延时
        }

        data->adc_value = adc_sum / samples;

        // 计算电压（mV -> V）
        uint32_t voltage_mv = esp_adc_cal_raw_to_voltage(data->adc_value, adc_chars);
        data->voltage = voltage_mv / 1000.0f;

        // 计算光照强度百分比
        // 注意：5516模块的电路设计是 VCC → 光敏电阻 → AO → 下拉电阻 → GND
        // 因此：光照强时，电阻小，AO电压低，ADC值低
        //      光照弱时，电阻大，AO电压高，ADC值高
        // 所以需要反转计算：光照强度 = 100% - (ADC值 / 4095) * 100%
        data->light_intensity = 100.0f - (data->adc_value / 4095.0f) * 100.0f;

        // 获取光照等级
        data->light_level = get_light_level(data->light_intensity);
    } else {
        data->adc_value = 0;
        data->voltage = 0.0f;
        data->light_intensity = 0.0f;
        data->light_level = LIGHT_LEVEL_DARK;
    }

    // 记录时间戳
    data->timestamp_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;

    return ESP_OK;
}

esp_err_t ldr_driver_deinit(void)
{
    if (!s_initialized) {
        ESP_LOGW(TAG, "⚠️ 驱动未初始化");
        return ESP_OK;
    }

    // 释放ADC校准数据
    if (adc_chars != NULL) {
        free(adc_chars);
        adc_chars = NULL;
    }

    s_initialized = false;
    s_do_gpio = -1;
    s_adc_channel = -1;

    ESP_LOGI(TAG, "✅ 5516光敏电阻驱动已反初始化");

    return ESP_OK;
}

const char* ldr_get_level_string(light_level_t level)
{
    switch (level) {
        case LIGHT_LEVEL_DARK:
            return "黑暗";
        case LIGHT_LEVEL_DIM:
            return "昏暗";
        case LIGHT_LEVEL_MODERATE:
            return "适中";
        case LIGHT_LEVEL_BRIGHT:
            return "明亮";
        case LIGHT_LEVEL_VERY_BRIGHT:
            return "非常明亮";
        default:
            return "未知";
    }
}
