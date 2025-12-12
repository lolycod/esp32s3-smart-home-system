/**
 * @file dht11_driver.c
 * @brief DHT11温湿度传感器驱动层实现（ESP32平台）
 */

#include "dht11_driver.h"
#include "driver_dht11.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_rom_sys.h"

static const char *TAG = "DHT11_DRIVER";

// DHT11句柄
static dht11_handle_t s_dht11_handle;
static uint8_t s_gpio_num = 0;
static bool s_initialized = false;

// 临界区互斥锁
static portMUX_TYPE s_dht11_spinlock = portMUX_INITIALIZER_UNLOCKED;

/**
 * @brief GPIO总线初始化
 */
static uint8_t dht11_bus_init(void)
{
    // 配置GPIO为开漏输出+输入模式（单总线协议标准实现）
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << s_gpio_num),
        .mode = GPIO_MODE_INPUT_OUTPUT_OD,  // 开漏+输入使能
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    if (gpio_config(&io_conf) != ESP_OK) {
        ESP_LOGE(TAG, "GPIO配置失败");
        return 1;
    }

    // 拉高总线（实际上是高阻态，由上拉电阻保持高电平）
    gpio_set_level(s_gpio_num, 1);

    // 等待DHT11稳定
    vTaskDelay(pdMS_TO_TICKS(2000));

    return 0;
}

/**
 * @brief GPIO总线反初始化
 */
static uint8_t dht11_bus_deinit(void)
{
    gpio_reset_pin(s_gpio_num);
    return 0;
}

/**
 * @brief GPIO总线读取
 */
static uint8_t dht11_bus_read(uint8_t *value)
{
    *value = gpio_get_level(s_gpio_num);
    return 0;
}

/**
 * @brief GPIO总线写入
 *
 * 开漏模式实现：
 * - 写0：FET导通，强拉低总线
 * - 写1：FET断开，高阻态，上拉电阻拉高
 */
static uint8_t dht11_bus_write(uint8_t value)
{
    gpio_set_level(s_gpio_num, value);
    return 0;
}

/**
 * @brief 毫秒延时
 */
static void dht11_delay_ms(uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}

/**
 * @brief 微秒延时
 */
static void dht11_delay_us(uint32_t us)
{
    esp_rom_delay_us(us);
}

/**
 * @brief 使能中断（退出临界区）
 */
static void dht11_enable_irq(void)
{
    taskEXIT_CRITICAL(&s_dht11_spinlock);
}

/**
 * @brief 禁止中断（进入临界区）
 */
static void dht11_disable_irq(void)
{
    taskENTER_CRITICAL(&s_dht11_spinlock);
}

/**
 * @brief 调试打印
 */
static void dht11_debug_print(const char *const fmt, ...)
{
    // 使用ESP_LOGI打印调试信息
    if (fmt != NULL) {
        ESP_LOGD(TAG, "%s", fmt);
    }
}


/**
 * @brief 初始化DHT11驱动
 */
esp_err_t dht11_driver_init(const dht11_driver_config_t *config)
{
    if (config == NULL) {
        ESP_LOGE(TAG, "配置参数为空");
        return ESP_ERR_INVALID_ARG;
    }

    if (s_initialized) {
        ESP_LOGW(TAG, "DHT11驱动已初始化");
        return ESP_OK;
    }

    s_gpio_num = config->gpio_num;

    // 初始化DHT11句柄
    DRIVER_DHT11_LINK_INIT(&s_dht11_handle, dht11_handle_t);
    DRIVER_DHT11_LINK_BUS_INIT(&s_dht11_handle, dht11_bus_init);
    DRIVER_DHT11_LINK_BUS_DEINIT(&s_dht11_handle, dht11_bus_deinit);
    DRIVER_DHT11_LINK_BUS_READ(&s_dht11_handle, dht11_bus_read);
    DRIVER_DHT11_LINK_BUS_WRITE(&s_dht11_handle, dht11_bus_write);
    DRIVER_DHT11_LINK_DELAY_MS(&s_dht11_handle, dht11_delay_ms);
    DRIVER_DHT11_LINK_DELAY_US(&s_dht11_handle, dht11_delay_us);
    DRIVER_DHT11_LINK_ENABLE_IRQ(&s_dht11_handle, dht11_enable_irq);
    DRIVER_DHT11_LINK_DISABLE_IRQ(&s_dht11_handle, dht11_disable_irq);
    DRIVER_DHT11_LINK_DEBUG_PRINT(&s_dht11_handle, dht11_debug_print);

    // 初始化DHT11芯片
    uint8_t res = dht11_init(&s_dht11_handle);
    if (res != 0) {
        ESP_LOGE(TAG, "DHT11初始化失败，错误码=%d", res);
        return ESP_FAIL;
    }

    s_initialized = true;
    ESP_LOGI(TAG, "✅ DHT11驱动初始化成功，GPIO%d", s_gpio_num);

    return ESP_OK;
}

/**
 * @brief 读取温湿度数据
 */
esp_err_t dht11_driver_read(dht11_data_t *data)
{
    if (!s_initialized) {
        ESP_LOGE(TAG, "DHT11驱动未初始化");
        return ESP_ERR_INVALID_STATE;
    }

    if (data == NULL) {
        ESP_LOGE(TAG, "数据指针为空");
        return ESP_ERR_INVALID_ARG;
    }

    // 读取温湿度
    uint8_t res = dht11_read_temperature_humidity(
        &s_dht11_handle,
        &data->temperature_raw,
        &data->temperature,
        &data->humidity_raw,
        &data->humidity
    );

    if (res != 0) {
        ESP_LOGW(TAG, "DHT11读取失败，错误码=%d", res);
        return ESP_FAIL;
    }

    ESP_LOGD(TAG, "DHT11读取成功: 温度=%.1f°C, 湿度=%d%%",
             data->temperature, data->humidity);

    return ESP_OK;
}

/**
 * @brief 反初始化DHT11驱动
 */
esp_err_t dht11_driver_deinit(void)
{
    if (!s_initialized) {
        return ESP_OK;
    }

    uint8_t res = dht11_deinit(&s_dht11_handle);
    if (res != 0) {
        ESP_LOGE(TAG, "DHT11反初始化失败");
        return ESP_FAIL;
    }

    s_initialized = false;
    ESP_LOGI(TAG, "DHT11驱动已关闭");

    return ESP_OK;
}
