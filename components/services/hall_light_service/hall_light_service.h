/**
 * @file hall_light_service.h
 * @brief 大厅灯服务（使用ESP32S3内置RGB灯）
 *
 * 功能：
 * - 控制ESP32S3内置RGB灯作为大厅灯
 * - 支持开关控制
 * - 提供固定的温馨颜色
 */

#ifndef HALL_LIGHT_SERVICE_H
#define HALL_LIGHT_SERVICE_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 大厅灯配置
 */
typedef struct {
    int gpio_pin;         ///< RGB灯GPIO引脚（ESP32S3通常是GPIO48）
    uint8_t brightness;   ///< 亮度 0-100
} hall_light_config_t;

/**
 * @brief 初始化大厅灯服务
 *
 * @param config 配置参数
 * @return esp_err_t
 */
esp_err_t hall_light_init(const hall_light_config_t *config);

/**
 * @brief 开启大厅灯
 *
 * @return esp_err_t
 */
esp_err_t hall_light_on(void);

/**
 * @brief 关闭大厅灯
 *
 * @return esp_err_t
 */
esp_err_t hall_light_off(void);

/**
 * @brief 切换大厅灯状态
 *
 * @return esp_err_t
 */
esp_err_t hall_light_toggle(void);

/**
 * @brief 获取大厅灯状态
 *
 * @return true=开启, false=关闭
 */
bool hall_light_is_on(void);

/**
 * @brief 设置大厅灯亮度
 *
 * @param brightness 亮度 0-100
 * @return esp_err_t
 */
esp_err_t hall_light_set_brightness(uint8_t brightness);

/**
 * @brief 反初始化大厅灯服务
 *
 * @return esp_err_t
 */
esp_err_t hall_light_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // HALL_LIGHT_SERVICE_H
