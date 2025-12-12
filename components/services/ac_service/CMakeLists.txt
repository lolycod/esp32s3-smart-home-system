idf_component_register(
    SRCS "ac_service.c"
    INCLUDE_DIRS "."
    REQUIRES sensor_service rgb_pwm_driver
)
