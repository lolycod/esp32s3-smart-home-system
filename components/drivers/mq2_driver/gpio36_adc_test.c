/**
 * @file gpio36_adc_test.c
 * @brief GPIO36 ADC测试（用于验证MQ-2模拟输出）
 */

#include "driver/adc.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_adc_cal.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "ADC_TEST";

static esp_adc_cal_characteristics_t *adc_chars;

/**
 * @brief 测试GPIO36 ADC通道
 */
void test_gpio36_adc_only(void)
{
    ESP_LOGI(TAG, "========== GPIO36 ADC通道测试 ==========");

    // 初始化ADC
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);  // GPIO36

    // 校准ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 3300, adc_chars);

    ESP_LOGI(TAG, "GPIO36 ADC已初始化");
    ESP_LOGI(TAG, "开始连续10次读取...");

    for (int i = 0; i < 10; i++) {
        int raw = adc1_get_raw(ADC1_CHANNEL_0);
        uint32_t mv = esp_adc_cal_raw_to_voltage(raw, adc_chars);
        float voltage = (float)mv / 1000.0f;
        float concentration = (voltage / 3.3f) * 100.0f;

        ESP_LOGI(TAG, "[%d] RAW: %d, 电压: %.3fV, 浓度: %.1f%%",
                 i, raw, voltage, concentration);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    ESP_LOGI(TAG, "========== 测试完成 ==========");

    free(adc_chars);

    // 根据测试结果给出诊断
    if (voltage < 0.1f) {
        ESP_LOGE(TAG, "🔴 诊断：GPIO36读取为0V！");
        ESP_LOGE(TAG, "   可能原因：");
        ESP_LOGE(TAG, "   1. AOUT引脚未连接到GPIO36");
        ESP_LOGE(TAG, "   2. MQ-2模块未供电或供电不足");
        ESP_LOGE(TAG, "   3. 连接线松动");
        ESP_LOGE(TAG, "   4. GPIO36引脚损坏");
    } else if (voltage < 0.3f) {
        ESP_LOGW(TAG, "🟡 诊断：GPIO36有信号但电压很低");
        esp_LOGW(TAG, "   可能：MQ-2处于无烟雾状态，这是正常的");
    } else {
        ESP_LOGI(TAG, "✅ 诊断：GPIO36工作正常！");
    }
}