/**
 * @file wifi_service.c
 * @brief WiFi连接管理服务实现
 */

#include "wifi_service.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include <string.h>

static const char *TAG = "WIFI_SERVICE";

// WiFi事件标志位
#define WIFI_CONNECTED_BIT    BIT0
#define WIFI_FAIL_BIT         BIT1

// 静态变量
static EventGroupHandle_t s_wifi_event_group = NULL;
static esp_netif_t *s_sta_netif = NULL;
static wifi_service_config_t s_wifi_config = {0};
static int s_retry_num = 0;
static bool s_is_connected = false;

/**
 * @brief WiFi事件处理器
 */
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "WiFi驱动已启动，开始连接...");
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < s_wifi_config.max_retry) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "WiFi连接失败，正在重试 (%d/%d)...", s_retry_num, s_wifi_config.max_retry);
        } else {
            ESP_LOGE(TAG, "WiFi连接失败，已达最大重试次数");
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        s_is_connected = false;
        ESP_LOGW(TAG, "WiFi连接断开");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "✅ 获得IP地址: " IPSTR, IP2STR(&event->ip_info.ip));
        ESP_LOGI(TAG, "   网关: " IPSTR, IP2STR(&event->ip_info.gw));
        ESP_LOGI(TAG, "   子网掩码: " IPSTR, IP2STR(&event->ip_info.netmask));
        s_retry_num = 0;
        s_is_connected = true;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

/**
 * @brief 初始化WiFi服务
 */
esp_err_t wifi_service_init(const wifi_service_config_t *config)
{
    if (config == NULL) {
        ESP_LOGE(TAG, "配置参数为空");
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "========== 初始化WiFi服务 ==========");

    // 保存配置
    memcpy(&s_wifi_config, config, sizeof(wifi_service_config_t));

    // 创建事件组
    s_wifi_event_group = xEventGroupCreate();
    if (s_wifi_event_group == NULL) {
        ESP_LOGE(TAG, "创建事件组失败");
        return ESP_FAIL;
    }

    // 初始化网络接口
    ESP_ERROR_CHECK(esp_netif_init());

    // 创建默认事件循环
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // 创建默认STA网络接口
    s_sta_netif = esp_netif_create_default_wifi_sta();
    if (s_sta_netif == NULL) {
        ESP_LOGE(TAG, "创建STA网络接口失败");
        return ESP_FAIL;
    }

    // WiFi驱动配置
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // 注册WiFi事件处理器
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                               &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                               &wifi_event_handler, NULL));

    // 配置WiFi
    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };

    strncpy((char *)wifi_config.sta.ssid, s_wifi_config.ssid, sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char *)wifi_config.sta.password, s_wifi_config.password, sizeof(wifi_config.sta.password) - 1);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    ESP_LOGI(TAG, "✅ WiFi服务初始化完成");
    ESP_LOGI(TAG, "   SSID: %s", s_wifi_config.ssid);
    ESP_LOGI(TAG, "   最大重试次数: %d", s_wifi_config.max_retry);
    ESP_LOGI(TAG, "========================================");

    return ESP_OK;
}

/**
 * @brief 连接WiFi
 */
esp_err_t wifi_service_connect(void)
{
    if (s_wifi_event_group == NULL) {
        ESP_LOGE(TAG, "WiFi服务未初始化");
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "🌐 开始连接WiFi...");

    // 启动WiFi
    esp_err_t ret = esp_wifi_start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "WiFi启动失败: %s", esp_err_to_name(ret));
        return ret;
    }

    // 等待连接结果（这里不阻塞，让main.c的循环去检测）
    return ESP_OK;
}

/**
 * @brief 断开WiFi连接
 */
esp_err_t wifi_service_disconnect(void)
{
    ESP_LOGI(TAG, "断开WiFi连接");
    s_is_connected = false;
    return esp_wifi_disconnect();
}

/**
 * @brief 检查WiFi是否已连接
 */
bool wifi_service_is_connected(void)
{
    return s_is_connected;
}

/**
 * @brief 反初始化WiFi服务
 */
esp_err_t wifi_service_deinit(void)
{
    ESP_LOGI(TAG, "WiFi服务关闭");

    s_is_connected = false;

    // 注销事件处理器
    esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler);
    esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler);

    // 停止WiFi
    esp_wifi_stop();
    esp_wifi_deinit();

    // 删除网络接口
    if (s_sta_netif) {
        esp_netif_destroy(s_sta_netif);
        s_sta_netif = NULL;
    }

    // 删除事件组
    if (s_wifi_event_group) {
        vEventGroupDelete(s_wifi_event_group);
        s_wifi_event_group = NULL;
    }

    return ESP_OK;
}
