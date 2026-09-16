# Learning the Basics

> This folder contains the original incremental ESP32 learning project. The active SIMP processor integration work now lives in `../simp_processor_flow/`.

# Embedded Firmware ESP32

A hands-on learning project I started to strengthen my embedded C and firmware fundamentals on an ESP32. My main background is electrical engineering/VLSI, so this repository documents the project **incrementally** rather than presenting only the final code.

The project evolved from simple GPIO polling into interrupts, FreeRTOS task notifications, multitasking, I2C, and an SSD1306 OLED framebuffer.

<img width="1280" height="1183" alt="photo_ESP_32" src="https://github.com/user-attachments/assets/1099b09a-b23c-4a4c-94ad-490462f6caca" />

## Current Hardware

| Device | Connection |
| --- | --- |
| Push button | GPIO4, active-low using internal pull-up |
| Active buzzer | GPIO23 |
| SSD1306 OLED SDA | GPIO21 |
| SSD1306 OLED SCL | GPIO22 |
| OLED I2C address | `0x3C` |

## Current Behavior

```text
Button press
    |
    v
GPIO falling-edge interrupt
    |
    v
button ISR
    |
    | FreeRTOS task notification
    v
app_main task wakes
    |
    +--> debounce / verify press
    |
    +--> fill OLED white
    |
    +--> buzzer ON for 200 ms
    |
    +--> wait for button release
    |
    +--> clear OLED to black
```

A separate `background_task` prints once per second. It is mainly there to demonstrate an important FreeRTOS idea: **a blocked task is not the same as a blocked CPU**.

## What I Am Learning

- GPIO input and output
- Active-low signals and internal pull-up resistors
- Mechanical switch bounce and simple debouncing
- GPIO interrupts and interrupt service routines (ISRs)
- FreeRTOS tasks, blocking, and scheduling
- Task notifications from ISR context
- Creating a second task with `xTaskCreate()`
- Basic I2C communication
- SSD1306 OLED control
- 1-bit framebuffers and byte-level pixel storage
- Modular C using `.c` / `.h` interfaces
- ESP-IDF configuration structs, handles, and driver APIs

## Repository Structure

```text
Embedded-Firmware-ESP32/
|
+-- CMakeLists.txt
+-- README.md
|
+-- main/
|   +-- CMakeLists.txt
|   +-- main.c                    # current application
|
+-- components/
|   +-- button/
|   |   +-- CMakeLists.txt
|   |   +-- button.c
|   |   +-- include/button.h
|   |
|   +-- buzzer/
|   |   +-- CMakeLists.txt
|   |   +-- buzzer.c
|   |   +-- include/buzzer.h
|   |
|   +-- oled/
|       +-- CMakeLists.txt
|       +-- oled.c
|       +-- include/oled.h
|
+-- learning_stages/              # historical snapshots; not compiled
|   +-- 01_gpio_polling.c
|   +-- 02_button_bounce_test.c
|   +-- 03_interrupt_flag.c
|   +-- 04_freertos_task_notification.c
|   +-- 05_oled_integration.c
|   +-- README.md
|
+-- docs/
    +-- buzzer_first_exercise.md
    +-- oled_flow.txt
    +-- checkpoints/
        +-- 01_gpio_button_buzzer.md
```

## Learning Progression

```text
GPIO polling
    |
    v
button bounce experiment
    |
    v
GPIO interrupt + software flag
    |
    v
FreeRTOS task notification
    |
    v
blocking + second task
    |
    v
I2C + SSD1306 OLED
```

The older versions are deliberately kept under [`learning_stages/`](learning_stages/) so the progression is visible without mixing multiple `app_main()` implementations into the active build.

## Current Source Modules

### `button`
Configures GPIO4 as an active-low input with an internal pull-up, registers a falling-edge ISR, and uses a FreeRTOS task notification to wake the task that initialized the module. Debouncing and waiting for release happen in normal task context rather than inside the ISR.

### `buzzer`
Small GPIO output module for the active buzzer on GPIO23.

### `oled`
Configures the I2C bus and SSD1306 panel, stores the 128x64 1-bit image in a 1024-byte framebuffer, and exposes simple `oled_fill()` / `oled_clear()` functions to the application.

## Build

This is organized as an ESP-IDF project. With ESP-IDF installed and its environment loaded:

```bash
idf.py set-target esp32
idf.py build
idf.py flash monitor
```

The exact OLED driver APIs are tied to the ESP-IDF version used by the project, so if building with a substantially different ESP-IDF release, check the corresponding `esp_lcd` / I2C API documentation.

## Notes

This is primarily a learning repository. The comments are intentionally more detailed than production firmware because they document the concepts I was learning while implementing each stage.
