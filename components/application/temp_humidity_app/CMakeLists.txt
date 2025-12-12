idf_component_register(
    SRCS "temp_humidity_app.c"
    INCLUDE_DIRS "."
    REQUIRES sensor_service ac_service
)
