idf_component_register(
    SRCS "dht11_driver.c"
         "dht11/src/driver_dht11.c"
         "gpio_test.c"
    INCLUDE_DIRS "."
                 "dht11/src"
    REQUIRES driver
)
