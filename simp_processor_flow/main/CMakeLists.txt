idf_component_register(
    SRCS
        "main.c"
        "button.c"
        "buzzer.c"
        "oled.c"
        "simp.c"

    PRIV_REQUIRES
        esp_driver_gpio
        esp_driver_i2c
        esp_lcd

    INCLUDE_DIRS "."
)