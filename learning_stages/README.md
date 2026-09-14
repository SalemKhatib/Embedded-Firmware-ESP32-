# Learning Stages

These files are historical checkpoints from the project and are intentionally **not** part of the active ESP-IDF build.

The current build uses `../main/main.c` together with the reusable components under `../components/`.

## Progression

1. `01_gpio_polling.c`  
   Basic polling: read the active-low button and directly control the buzzer.

2. `02_button_bounce_test.c`  
   Small experiment used to observe mechanical button bounce and motivate debouncing.

3. `03_interrupt_flag.c`  
   Historical interrupt + software-flag version. This snapshot used an older button-module API (`button_event_pending()` / `button_clear_event()`) that was later replaced, so it is kept for learning history rather than compilation with the current component.

4. `04_freertos_task_notification.c`  
   Replaced the software flag with a FreeRTOS task notification, added blocking behavior, and created a second background task to demonstrate that a blocked task does not block the CPU.

5. `05_oled_integration.c`  
   Added the SSD1306 OLED over I2C. A valid press fills the OLED, beeps the buzzer, waits for release, and clears the display.

## Why keep these files?

The repository is intentionally incremental. The checkpoints show how the same hardware exercise evolved from simple GPIO polling into interrupts, FreeRTOS task synchronization, multitasking, and I2C display control.
