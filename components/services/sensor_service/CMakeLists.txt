idf_component_register(
    SRCS "sensor_service.c"
    INCLUDE_DIRS "."
    REQUIRES dht_driver mq2_driver ldr_driver esp_timer websocket_service
)
