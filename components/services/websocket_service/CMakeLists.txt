# WebSocket服务组件依赖

idf_component_register(
    SRCS "websocket_service.c"
    INCLUDE_DIRS "."
    REQUIRES sensor_service espressif__esp_websocket_client ac_service hall_light_service motor_service servo_driver json
    PRIV_REQUIRES esp_event
)
