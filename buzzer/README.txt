# 01 - GPIO Buzzer

My first GPIO output exercise using ESP32 and ESP-IDF in C.

## Hardware

- ESP32 development board
- TMB12A03 active buzzer
- Buzzer positive pin connected to GPIO23
- Buzzer negative pin connected to GND

## Wiring

GPIO23 ---- (+) Buzzer
GND ------- (-) Buzzer

## What I learned

- Configuring a GPIO as an output
- Setting a GPIO HIGH and LOW
- Using FreeRTOS `vTaskDelay()`
- Converting milliseconds to scheduler ticks with `pdMS_TO_TICKS()`
- Understanding that ESP-IDF runs on FreeRTOS

## Behavior

The buzzer repeatedly:

1. Turns on for 500 ms
2. Turns off for 500 ms
3. Repeats

## Important functions

```c
gpio_reset_pin()
gpio_set_direction()
gpio_set_level()
vTaskDelay()
pdMS_TO_TICKS()