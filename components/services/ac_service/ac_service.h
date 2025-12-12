/**
 * @file ac_service.h
 * @brief 智能空调控制服务
 *
 * 功能：
 * - 根据温度、湿度、烟雾浓度判断环境舒适度
 * - 自动控制RGB灯模拟空调压缩机状态：
 *   绿色：环境舒适
 *   红色（PWM深度）：需要制热，颜色越深温度越低
 *   蓝色（PWM深度）：需要制冷，颜色越深温度越高
 */

#ifndef AC_SERVICE_H
#define AC_SERVICE_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "sensor_service.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 空调模式
 */
typedef enum {
    AC_MODE_OFF = 0,      ///< 关闭
    AC_MODE_COMFORT,      ///< 舒适模式（绿色）
    AC_MODE_HEATING,      ///< 制热模式（红色）
    AC_MODE_COOLING       ///< 制冷模式（蓝色）
} ac_mode_t;

/**
 * @brief 空调状态
 */
typedef struct {
    ac_mode_t mode;        ///< 当前模式
    uint8_t intensity;     ///< 强度0-100（PWM深度）
    float target_temp;     ///< 目标温度
    bool auto_mode;        ///< 是否自动模式
} ac_status_t;

/**
 * @brief 舒适区配置
 */
typedef struct {
    float temp_min;        ///< 最低舒适温度（°C）
    float temp_max;        ///< 最高舒适温度（°C）
    int humidity_min;      ///< 最低舒适湿度（%）
    int humidity_max;      ///< 最高舒适湿度（%）
    float smoke_threshold; ///< 烟雾浓度阈值（V）
    int gpio_r;            ///< RGB灯红色通道GPIO
    int gpio_g;            ///< RGB灯绿色通道GPIO
    int gpio_b;            ///< RGB灯蓝色通道GPIO
} ac_service_config_t;

/**
 * @brief 初始化空调服务
 *
 * @param config 配置参数
 * @return esp_err_t
 */
esp_err_t ac_service_init(const ac_service_config_t *config);

/**
 * @brief 更新空调状态（根据传感器数据）
 *
 * @param sensor_data 传感器数据
 * @return esp_err_t
 */
esp_err_t ac_service_update(const sensor_data_t *sensor_data);

/**
 * @brief 获取当前空调状态
 *
 * @param status 状态输出
 * @return esp_err_t
 */
esp_err_t ac_service_get_status(ac_status_t *status);

/**
 * @brief 手动设置空调模式
 *
 * @param mode 模式
 * @param intensity 强度0-100
 * @return esp_err_t
 */
esp_err_t ac_service_set_mode(ac_mode_t mode, uint8_t intensity);

/**
 * @brief 启用/禁用自动模式
 *
 * @param enable true=自动, false=手动
 * @return esp_err_t
 */
esp_err_t ac_service_set_auto(bool enable);

/**
 * @brief 设置目标温度
 *
 * @param target_temp 目标温度
 * @return esp_err_t
 */
esp_err_t ac_service_set_target_temp(float target_temp);

/**
 * @brief 设置智能灯开关（控制LED显示）
 *
 * @param enabled true=开灯（显示颜色）, false=关灯（全黑）
 * @return esp_err_t
 */
esp_err_t ac_service_set_led_enabled(bool enabled);

/**
 * @brief 获取智能灯开关状态
 *
 * @return bool true=开灯, false=关灯
 */
bool ac_service_is_led_enabled(void);

/**
 * @brief 反初始化空调服务
 *
 * @return esp_err_t
 */
esp_err_t ac_service_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // AC_SERVICE_H
