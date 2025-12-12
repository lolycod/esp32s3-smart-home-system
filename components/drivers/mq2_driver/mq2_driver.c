/**
 * @file mq2_driver.c
 * @brief MQ-2烟雾传感器驱动实现
 */

#include "mq2_driver.h"

#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "MQ2_DRIVER";

// 配置参数
static int s_ttl_gpio = -1;
static int s_adc_channel = -1;
static adc1_channel_t s_adc1_channel_enum = ADC1_CHANNEL_0;  // ⭐存储正确的枚举值
static bool s_initialized = false;

// ADC配置
static esp_adc_cal_characteristics_t *adc_chars;
static const adc_bits_width_t adc_width = ADC_WIDTH_BIT_12;
static const adc_atten_t adc_atten = ADC_ATTEN_DB_11;  // 11dB衰减

/**
 * @brief 初始化GPIO（TTL输出）
 */
static esp_err_t init_ttl_gpio(int gpio_num)
{
    if (gpio_num < 0) {
        // 跳过TTL初始化
        return ESP_OK;
    }

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << gpio_num),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,   // 启用上拉
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    return gpio_config(&io_conf);
}

/**
 * @brief 初始化ADC（模拟量输入）
 * 
 * ESP32-S3 ADC1通道映射:
 * - ADC1_CH3 → GPIO4
 * - ADC1_CH4 → GPIO5
 * - ADC1_CH5 → GPIO6 (推荐用于MQ-2)
 * - ADC1_CH6 → GPIO7
 * - ADC1_CH7 → GPIO8
 * - ADC1_CH8 → GPIO9
 * - ADC1_CH9 → GPIO10
 */
static esp_err_t init_adc(int channel)
{
    if (channel < 0) {
        // 跳过ADC初始化
        return ESP_OK;
    }

    // 验证ADC通道范围(ESP32-S3 ADC1支持CH0-CH9对应GPIO1-10)
    if (channel < 0 || channel > 9) {
        ESP_LOGE(TAG, "❌ 无效的ADC通道: %d", channel);
        ESP_LOGE(TAG, "ESP32-S3 ADC1支持的通道:");
        ESP_LOGE(TAG, "  - 通道0 (GPIO1)");
        ESP_LOGE(TAG, "  - 通道1 (GPIO2)");
        ESP_LOGE(TAG, "  - 通道2 (GPIO3)");
        ESP_LOGE(TAG, "  - 通道3 (GPIO4) ⚠️被DHT11占用");
        ESP_LOGE(TAG, "  - 通道4 (GPIO5)");
        ESP_LOGE(TAG, "  - 通道5 (GPIO6) ⭐推荐用于MQ-2");
        ESP_LOGE(TAG, "  - 通道6 (GPIO7)");
        ESP_LOGE(TAG, "  - 通道7 (GPIO8)");
        ESP_LOGE(TAG, "  - 通道8 (GPIO9)");
        ESP_LOGE(TAG, "  - 通道9 (GPIO10)");
        return ESP_ERR_INVALID_ARG;
    }

    // ESP32-S3 ADC1通道映射
    // ⭐关键: ESP32-S3的ADC通道号和GPIO的对应关系
    adc1_channel_t adc_channel;
    int gpio_num = -1;
    
    switch (channel) {
        case 0:  // ADC1_CH0 对应 GPIO1
            adc_channel = ADC1_CHANNEL_0;
            gpio_num = 1;
            break;
        case 1:  // ADC1_CH1 对应 GPIO2
            adc_channel = ADC1_CHANNEL_1;
            gpio_num = 2;
            break;
        case 2:  // ADC1_CH2 对应 GPIO3
            adc_channel = ADC1_CHANNEL_2;
            gpio_num = 3;
            break;
        case 3:  // ADC1_CH3 对应 GPIO4
            adc_channel = ADC1_CHANNEL_3;
            gpio_num = 4;
            break;
        case 4:  // ADC1_CH4 对应 GPIO5
            adc_channel = ADC1_CHANNEL_4;
            gpio_num = 5;
            break;
        case 5:  // ADC1_CH5 对应 GPIO6 ⭐推荐用于MQ-2
            adc_channel = ADC1_CHANNEL_5;
            gpio_num = 6;
            break;
        case 6:  // ADC1_CH6 对应 GPIO7
            adc_channel = ADC1_CHANNEL_6;
            gpio_num = 7;
            break;
        case 7:  // ADC1_CH7 对应 GPIO8
            adc_channel = ADC1_CHANNEL_7;
            gpio_num = 8;
            break;
        case 8:  // ADC1_CH8 对应 GPIO9
            adc_channel = ADC1_CHANNEL_8;
            gpio_num = 9;
            break;
        case 9:  // ADC1_CH9 对应 GPIO10
            adc_channel = ADC1_CHANNEL_9;
            gpio_num = 10;
            break;
        default:
            ESP_LOGE(TAG, "不支持的ADC通道: %d", channel);
            return ESP_ERR_INVALID_ARG;
    }

    // ⭐关键修复: 存储正确的ADC枚举值
    s_adc1_channel_enum = adc_channel;

    ESP_LOGI(TAG, "⭐ 通道映射: channel=%d → ADC1_CHANNEL_%d → GPIO%d",
             channel, channel, gpio_num);

    ESP_LOGI(TAG, "配置ADC1_CH%d → GPIO%d", channel, gpio_num);

    // ⭐关键修复: ESP32-S3必须先配置ADC宽度和衰减，再初始化GPIO
    ESP_LOGI(TAG, "步骤1: 配置ADC1宽度");
    esp_err_t ret = adc1_config_width(adc_width);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "❌ ADC1配置宽度失败: %d", ret);
        return ret;
    }
    ESP_LOGI(TAG, "✅ ADC1宽度: 12位(0-4095)");

    ESP_LOGI(TAG, "步骤2: 配置ADC1通道%d衰减", channel);
    ret = adc1_config_channel_atten(adc_channel, adc_atten);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "❌ ADC1通道衰减配置失败: %d", ret);
        return ret;
    }
    ESP_LOGI(TAG, "✅ ADC1衰减: 11dB(0-3.3V)");

    // ⭐关键诊断: 立即测试ADC读取（零延迟）
    ESP_LOGI(TAG, "步骤3: 零延迟ADC测试");
    ESP_LOGI(TAG, "🔍 立即读取ADC（无延迟、无校准）");
    int immediate_raw = adc1_get_raw(adc_channel);
    ESP_LOGI(TAG, "🔍 原始ADC值: %d (0x%X)", immediate_raw, immediate_raw);

    if (immediate_raw < 0) {
        ESP_LOGE(TAG, "❌ ADC读取失败，错误码: %d", immediate_raw);
        ESP_LOGE(TAG, "   这说明ADC通道配置有问题！");
        return ESP_FAIL;
    } else if (immediate_raw == 0) {
        ESP_LOGW(TAG, "⚠️ ADC读取为0");
        ESP_LOGW(TAG, "   可能原因: GPIO%d无输入信号或接地", gpio_num);
    } else {
        ESP_LOGI(TAG, "✅ ADC硬件工作正常！读取到非零值: %d", immediate_raw);
    }

    // 步骤4: 校准ADC
    ESP_LOGI(TAG, "步骤4: 校准ADC");
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    if (adc_chars == NULL) {
        ESP_LOGE(TAG, "❌ ADC校准结构体内存分配失败");
        return ESP_ERR_NO_MEM;
    }

    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(
        ADC_UNIT_1,
        adc_atten,
        adc_width,
        1100,  // 默认Vref (mV)
        adc_chars
    );

    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        ESP_LOGI(TAG, "✅ ADC校准: eFuse Two Point(最精确)");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        ESP_LOGI(TAG, "✅ ADC校准: eFuse Vref(较精确)");
    } else {
        ESP_LOGI(TAG, "⚠️ ADC校准: 默认Vref(精度较低)");
    }

    ESP_LOGI(TAG, "========== ADC初始化完成 ==========");
    return ESP_OK;
}

/**
 * @brief 初始化MQ-2驱动
 */
esp_err_t mq2_driver_init(const mq2_driver_config_t *config)
{
    if (config == NULL) {
        ESP_LOGE(TAG, "配置参数为空");
        return ESP_ERR_INVALID_ARG;
    }

    if (s_initialized) {
        ESP_LOGW(TAG, "MQ-2驱动已初始化");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "MQ-2配置信息:");
    ESP_LOGI(TAG, "  - TTL输出引脚: GPIO%d", config->ttl_gpio);
    ESP_LOGI(TAG, "  - ADC通道: %d", config->adc_channel);
    ESP_LOGI(TAG, "  - 采样间隔: %lu ms", config->sample_interval_ms);

    // 验证并打印ADC通道映射
    if (config->adc_channel >= 0) {
        int gpio_num = -1;
        switch (config->adc_channel) {
            case 3: gpio_num = 4; break;
            case 4: gpio_num = 5; break;
            case 5: gpio_num = 6; break;
            case 6: gpio_num = 7; break;
            case 7: gpio_num = 8; break;
            case 8: gpio_num = 9; break;
            case 9: gpio_num = 10; break;
            default:
                ESP_LOGE(TAG, "❌ 无效的ADC通道: %d", config->adc_channel);
                return ESP_ERR_INVALID_ARG;
        }
        ESP_LOGI(TAG, "ADC映射: ADC1_CH%d → GPIO%d", config->adc_channel, gpio_num);
    }

    // 初始化TTL GPIO
    s_ttl_gpio = config->ttl_gpio;
    esp_err_t ret = init_ttl_gpio(s_ttl_gpio);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "TTL GPIO初始化失败");
        return ret;
    }

    // 初始化ADC
    s_adc_channel = config->adc_channel;
    ret = init_adc(s_adc_channel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "ADC通道初始化失败");
        return ret;
    }

    // 稳定时间
    ESP_LOGI(TAG, "等待传感器预热（10秒）...");
    vTaskDelay(pdMS_TO_TICKS(10000));  // MQ-2需要预热

    // 执行ADC测试读取(多次采样)
    if (s_adc_channel >= 3 && s_adc_channel <= 9) {
        ESP_LOGI(TAG, "========== ADC功能测试 ==========");
        ESP_LOGI(TAG, "执行5次ADC测试读取...");
        
        int gpio_num = (s_adc_channel == 3) ? 4 :
                       (s_adc_channel == 4) ? 5 :
                       (s_adc_channel == 5) ? 6 :
                       (s_adc_channel == 6) ? 7 :
                       (s_adc_channel == 7) ? 8 :
                       (s_adc_channel == 8) ? 9 : 10;
        
        bool all_zero = true;
        int max_raw = 0;
        int min_raw = 4095;
        uint32_t sum_raw = 0;
        
        for (int i = 0; i < 5; i++) {
            // ⭐关键修复: 使用存储的正确枚举值
            int test_raw = adc1_get_raw(s_adc1_channel_enum);

            if (test_raw < 0) {
                ESP_LOGE(TAG, "❌ ADC测试读取失败(第%d次): 错误码=%d", i+1, test_raw);
                ESP_LOGE(TAG, "   可能原因: ADC通道配置错误或硬件故障");
                return ESP_FAIL;
            }
            
            uint32_t test_mv = esp_adc_cal_raw_to_voltage(test_raw, adc_chars);
            float test_v = (float)test_mv / 1000.0f;
            
            ESP_LOGI(TAG, "  [%d/5] RAW=%d, 电压=%.3fV (%lumV)", 
                     i+1, test_raw, test_v, test_mv);
            
            if (test_raw > 0) all_zero = false;
            if (test_raw > max_raw) max_raw = test_raw;
            if (test_raw < min_raw) min_raw = test_raw;
            sum_raw += test_raw;
            
            vTaskDelay(pdMS_TO_TICKS(100));  // 100ms间隔
        }
        
        int avg_raw = sum_raw / 5;
        uint32_t avg_mv = esp_adc_cal_raw_to_voltage(avg_raw, adc_chars);
        float avg_v = (float)avg_mv / 1000.0f;
        
        ESP_LOGI(TAG, "========== ADC测试统计 ==========");
        ESP_LOGI(TAG, "平均值: RAW=%d, 电压=%.3fV", avg_raw, avg_v);
        ESP_LOGI(TAG, "最大值: RAW=%d", max_raw);
        ESP_LOGI(TAG, "最小值: RAW=%d", min_raw);
        ESP_LOGI(TAG, "波动范围: %d", max_raw - min_raw);
        
        // 详细诊断
        if (all_zero) {
            ESP_LOGE(TAG, "");
            ESP_LOGE(TAG, "🔴🔴🔴 严重错误: 所有ADC读取都为0! 🔴🔴🔴");
            ESP_LOGE(TAG, "");
            ESP_LOGE(TAG, "可能原因:");
            ESP_LOGE(TAG, "  1. ❌ MQ-2 AOUT未连接到GPIO%d", gpio_num);
            ESP_LOGE(TAG, "  2. ❌ MQ-2模块未供电(需要5V,不是3.3V)");
            ESP_LOGE(TAG, "  3. ❌ ADC通道配置错误(当前配置: ADC1_CH%d)", s_adc_channel);
            ESP_LOGE(TAG, "  4. ❌ GPIO%d被其他功能占用", gpio_num);
            ESP_LOGE(TAG, "  5. ❌ 接线松动或接触不良");
            ESP_LOGE(TAG, "");
            ESP_LOGE(TAG, "硬件检查步骤:");
            ESP_LOGE(TAG, "  1. 用万用表测量MQ-2的VCC引脚,应该是5V");
            ESP_LOGE(TAG, "  2. 用万用表测量MQ-2的AOUT引脚,应该有0.1-3.0V的电压");
            ESP_LOGE(TAG, "  3. 确认AOUT连接到ESP32的GPIO%d", gpio_num);
            ESP_LOGE(TAG, "  4. 检查杜邦线是否松动");
            ESP_LOGE(TAG, "");
            return ESP_FAIL;
        } else if (avg_raw < 50) {
            ESP_LOGW(TAG, "⚠️ ADC值很低(平均%d),可能原因:", avg_raw);
            ESP_LOGW(TAG, "  1. 传感器预热不足(建议等待30秒以上)");
            ESP_LOGW(TAG, "  2. 环境中无烟雾(这是正常的)");
            ESP_LOGW(TAG, "  3. 传感器灵敏度设置过低");
        } else if (avg_raw < 200) {
            ESP_LOGI(TAG, "ℹ️ ADC值较低(平均%d),这是正常的基线电压", avg_raw);
            ESP_LOGI(TAG, "   传感器在无烟雾环境下应该输出低电压");
            ESP_LOGI(TAG, "✅ ADC功能正常,传感器工作正常");
        } else {
            ESP_LOGI(TAG, "✅ ADC读取正常(平均%d),传感器工作正常", avg_raw);
            if (avg_raw > 1000) {
                ESP_LOGW(TAG, "⚠️ 检测到较高的ADC值,可能环境中有烟雾或气体");
            }
        }
        
        ESP_LOGI(TAG, "=====================================");
    }

    s_initialized = true;
    ESP_LOGI(TAG, "✅ MQ-2烟雾传感器初始化成功");
    ESP_LOGI(TAG, "=================================================");

    return ESP_OK;
}

/**
 * @brief 读取MQ-2传感器数据
 */
esp_err_t mq2_driver_read(mq2_data_t *data)
{
    if (data == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (!s_initialized) {
        ESP_LOGE(TAG, "MQ-2驱动未初始化");
        return ESP_ERR_INVALID_STATE;
    }

    // 读取TTL数字输出
    bool ttl_detected = false;
    if (s_ttl_gpio >= 0) {
        ttl_detected = (gpio_get_level(s_ttl_gpio) == 0);  // 低电平=检测到烟雾
    }

    // 读取ADC模拟量(多次采样平均)
    uint32_t adc_raw = 0;
    float voltage = 0.0f;
    float concentration = 0.0f;

    if (s_adc_channel >= 3 && s_adc_channel <= 9) {
        // 多次采样取平均值,减少噪声
        const int samples = 10;
        uint32_t adc_sum = 0;
        int valid_samples = 0;
        
        for (int i = 0; i < samples; i++) {
            // ⭐关键修复: 使用存储的正确枚举值
            int raw = adc1_get_raw(s_adc1_channel_enum);

            if (raw < 0) {
                ESP_LOGW(TAG, "⚠️ ADC读取失败(第%d次): %d", i+1, raw);
                continue;
            }
            
            adc_sum += raw;
            valid_samples++;
            
            // 采样间隔,避免读取冲突
            vTaskDelay(pdMS_TO_TICKS(2));
        }
        
        if (valid_samples == 0) {
            ESP_LOGE(TAG, "❌ 所有ADC采样都失败");
            return ESP_FAIL;
        }
        
        // 计算平均值
        adc_raw = adc_sum / valid_samples;

        // 转换为电压
        uint32_t mv = esp_adc_cal_raw_to_voltage(adc_raw, adc_chars);
        voltage = (float)mv / 1000.0f;

        // 计算浓度百分比（0-3.3V映射到0-100%）
        concentration = (voltage / 3.3f) * 100.0f;
        if (concentration > 100.0f) concentration = 100.0f;
        if (concentration < 0.0f) concentration = 0.0f;

        // 详细诊断日志
        ESP_LOGI(TAG, "📊 ADC读取: RAW=%lu (平均%d次), 电压=%.3fV (%lumV), 浓度=%.1f%%", 
                 adc_raw, valid_samples, voltage, mv, concentration);

        // 诊断异常情况
        if (adc_raw == 0) {
            ESP_LOGE(TAG, "🔴 ADC读取为0！可能原因：");
            int gpio_num = (s_adc_channel == 5) ? 6 : s_adc_channel;
            ESP_LOGE(TAG, "   1. MQ-2 AOUT未连接到GPIO%d", gpio_num);
            ESP_LOGE(TAG, "   2. MQ-2模块未供电(需要5V)");
            ESP_LOGE(TAG, "   3. ADC通道配置错误");
            ESP_LOGE(TAG, "   4. 接线松动或接触不良");
        } else if (adc_raw < 100) {
            ESP_LOGD(TAG, "ℹ️ ADC值较低(%lu),MQ-2处于无烟雾状态(正常)", adc_raw);
        }
    } else {
        ESP_LOGW(TAG, "⚠️ ADC通道未配置或无效: %d", s_adc_channel);
    }

    // 填充数据
    data->smoke_detected = ttl_detected;
    data->adc_value = adc_raw;
    data->voltage = voltage;
    data->concentration = concentration;
    data->timestamp_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;

    return ESP_OK;
}

/**
 * @brief 反初始化MQ-2驱动
 */
esp_err_t mq2_driver_deinit(void)
{
    if (!s_initialized) {
        return ESP_OK;
    }

    if (s_ttl_gpio >= 0) {
        gpio_reset_pin(s_ttl_gpio);
    }

    if (adc_chars) {
        free(adc_chars);
        adc_chars = NULL;
    }

    s_initialized = false;
    ESP_LOGI(TAG, "MQ-2驱动已反初始化");

    return ESP_OK;
}
