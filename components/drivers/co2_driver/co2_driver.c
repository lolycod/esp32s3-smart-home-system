/**
 * @file co2_driver.c
 * @brief JW01 二氧化碳传感器驱动实现
 * 
 * JW01 数据帧格式 (9字节):
 * | B1   | B2   | B3   | B4   | B5   | B6   | B7   | B8   | B9   |
 * |------|------|------|------|------|------|------|------|------|
 * | 0x2C | 0xE4 | TVOC高| TVOC低| CH2O高| CH2O低| CO2高 | CO2低 | 校验 |
 * 
 * CO2 浓度 = B7 * 256 + B8 (ppm)
 * 校验和 = (B1 + B2 + ... + B8) & 0xFF
 */

#include "co2_driver.h"
#include "driver/uart.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "CO2_DRIVER";

// JW01 数据帧常量 (适配新型号 6字节协议)
// 接收数据: 2c 02 ac 03 ff dc (Example: 940ppm)
// 格式: HEAD1(2C) LEN(02) DATA_L DATA_H ALWAYS_FF CHECKSUM
#define JW01_FRAME_SIZE     6       // 数据帧长度改为6
#define JW01_HEADER_1       0x2C    // 帧头字节1
#define JW01_HEADER_2       0x02    // 帧头字节2 (长度/类型)
#define JW01_BAUD_RATE      9600    // 波特率

// UART 配置
#define CO2_UART_BUF_SIZE   256     // UART 缓冲区大小
#define CO2_READ_TIMEOUT_MS 1500     // 读取超时 (ms) - 增加以适配低频发送

static int s_uart_num = -1;
static bool s_initialized = false;

/**
 * @brief 验证校验和
 */
static bool verify_checksum(const uint8_t *frame, size_t len)
{
    if (len != JW01_FRAME_SIZE) {
        return false;
    }
    
    uint8_t sum = 0;
    for (int i = 0; i < JW01_FRAME_SIZE - 1; i++) {
        sum += frame[i];
    }
    
    return (sum == frame[JW01_FRAME_SIZE - 1]);
}

/**
 * @brief 在缓冲区中查找有效帧
 */
static int find_valid_frame(const uint8_t *buffer, size_t len, uint8_t *frame_out)
{
    // 查找有效帧 (适配 2C 01 ... 和 2C 02 ... 两种变体)
    // 格式: 2C [TYPE] [LOW] [HIGH] FF [SUM]
    for (size_t i = 0; i + JW01_FRAME_SIZE <= len; i++) {
        // 查找帧头 0x2C
        if (buffer[i] == JW01_HEADER_1) {
            // 验证校验和 (Sum of bytes 0..4 == byte 5)
            uint8_t sum = 0;
            for (int k = 0; k < JW01_FRAME_SIZE - 1; k++) {
                sum += buffer[i + k];
            }
            
            if (sum == buffer[i + JW01_FRAME_SIZE - 1]) {
                memcpy(frame_out, &buffer[i], JW01_FRAME_SIZE);
                return i + JW01_FRAME_SIZE;  // 返回消耗的字节数
            }
        }
    }
    return -1;  // 未找到有效帧
}

esp_err_t co2_driver_init(const co2_driver_config_t *config)
{
    if (config == NULL) {
        ESP_LOGE(TAG, "配置参数为空");
        return ESP_ERR_INVALID_ARG;
    }
    
    if (s_initialized) {
        ESP_LOGW(TAG, "驱动已初始化");
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "初始化 JW01 CO2 传感器驱动");
    ESP_LOGI(TAG, "   TX GPIO: %d", config->uart_tx_gpio);
    ESP_LOGI(TAG, "   RX GPIO: %d", config->uart_rx_gpio);
    ESP_LOGI(TAG, "   UART: %d", config->uart_num);
    
    s_uart_num = config->uart_num;
    
    // UART 配置
    uart_config_t uart_config = {
        .baud_rate = JW01_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    
    // 安装 UART 驱动
    esp_err_t ret = uart_driver_install(s_uart_num, CO2_UART_BUF_SIZE, 0, 0, NULL, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "UART 驱动安装失败: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ret = uart_param_config(s_uart_num, &uart_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "UART 参数配置失败: %s", esp_err_to_name(ret));
        uart_driver_delete(s_uart_num);
        return ret;
    }
    
    // 设置 UART 引脚
    ret = uart_set_pin(s_uart_num, config->uart_tx_gpio, config->uart_rx_gpio, 
                       UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "UART 引脚设置失败: %s", esp_err_to_name(ret));
        uart_driver_delete(s_uart_num);
        return ret;
    }
    
    s_initialized = true;
    ESP_LOGI(TAG, "✅ JW01 CO2 传感器驱动初始化成功");
    
    return ESP_OK;
}

esp_err_t co2_driver_read(co2_data_t *data)
{
    if (!s_initialized) {
        ESP_LOGE(TAG, "驱动未初始化");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (data == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // 初始化输出
    memset(data, 0, sizeof(co2_data_t));
    data->valid = false;
    
    // 读取 UART 数据：16 字节足够包含 2-3 个完整帧，500ms 超时保证能收到数据
    uint8_t buffer[CO2_UART_BUF_SIZE];
    int len = uart_read_bytes(s_uart_num, buffer, 16, pdMS_TO_TICKS(500));
    
    if (len <= 0) {
        ESP_LOGW(TAG, "⚠️ UART 超时，未收到 CO2 数据 (len=%d)", len);
        return ESP_ERR_TIMEOUT;
    }
    
    ESP_LOGD(TAG, "📥 收到 %d 字节", len);
    
    // ESP_LOGD(TAG, "读取到 %d 字节数据", len); // 保持Debug级别，避免刷屏
    
    // 查找有效帧
    uint8_t frame[JW01_FRAME_SIZE];
    int consumed = find_valid_frame(buffer, len, frame);
    
    if (consumed < 0) {
        ESP_LOGW(TAG, "⚠️ 收到数据但未找到有效帧 (波特率或协议不匹配)");
        ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, len, ESP_LOG_WARN); // 打印收到的原始数据
        return ESP_ERR_NOT_FOUND;
    }
    
    // 解析数据 (6字节协议: 2C [TYPE] Low High FF Sum)
    // 根据日志 2c 01 de 03 ... (03DE = 990ppm) 推断为 Little Endian
    uint8_t data_l = frame[2];
    uint8_t data_h = frame[3];
    data->co2_ppm = (data_h << 8) | data_l;
    
    // 该协议不包含 TVOC 和 CH2O
    data->tvoc_ppb = 0;
    data->ch2o_ppb = 0;
    data->valid = true;
    
    ESP_LOGI(TAG, "🌿 CO2: %d ppm (Raw: %02X %02X, Type: %02X)", 
             data->co2_ppm, data_l, data_h, frame[1]);
    
    return ESP_OK;
}

esp_err_t co2_driver_deinit(void)
{
    if (!s_initialized) {
        return ESP_OK;
    }
    
    uart_driver_delete(s_uart_num);
    s_uart_num = -1;
    s_initialized = false;
    
    ESP_LOGI(TAG, "CO2 传感器驱动已关闭");
    return ESP_OK;
}
