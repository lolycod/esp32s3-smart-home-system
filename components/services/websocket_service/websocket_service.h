/**
 * @file websocket_service.h
 * @brief WebSocket客户端服务
 */

#ifndef WEBSOCKET_SERVICE_H
#define WEBSOCKET_SERVICE_H

#include "esp_err.h"
#include "sensor_service.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char *uri;                    ///< WebSocket服务器URI (如: ws://192.168.1.100:8080)
    uint32_t send_interval_ms;          ///< 数据发送间隔(ms)
} websocket_service_config_t;

/**
 * @brief 初始化WebSocket服务
 */
esp_err_t websocket_service_init(const websocket_service_config_t *config);

/**
 * @brief 发送传感器数据
 */
esp_err_t websocket_service_send_sensor_data(const sensor_data_t *data);

/**
 * @brief 检查WebSocket是否已连接
 * @return true 已连接，false 未连接
 */
bool websocket_service_is_connected(void);

/**
 * @brief 反初始化WebSocket服务
 */
esp_err_t websocket_service_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // WEBSOCKET_SERVICE_H
