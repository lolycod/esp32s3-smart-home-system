idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES
        nvs_flash
        sensor_service
        temp_humidity_app
        wifi_service
        ac_service
        hall_light_service
        motor_service
        motor_app
        servo_driver
        driver
)
