# ESP32 Embedded Firmware & Custom CPU Integration

ESP32 firmware project combining incremental embedded-systems exercises with a working integration of a **custom SIMP processor simulator**, **FreeRTOS**, GPIO interrupts, an SSD1306 OLED, and a buzzer.

The repository documents the progression from basic ESP32 peripheral control to executing **SIMP machine-code programs on the ESP32** and displaying their generated graphics on real hardware.

---

## Project Overview

This repository contains two connected parts:

### `learning_basics/`

Incremental exercises used to learn and understand:

- ESP32 GPIO
- Push-button polling
- Button debouncing
- GPIO interrupts
- FreeRTOS tasks
- FreeRTOS task notifications
- ISR-to-task synchronization
- Active buzzer control
- I2C
- SSD1306 OLED control
- Framebuffer manipulation

### `simp_processor_flow/`

The main integration project.

A previously developed **SIMP CPU simulator written in C** was adapted to run on the ESP32 and connected to physical peripherals.

The complete flow is now working:

```text
Physical Button
      |
      v
GPIO Interrupt
      |
      v
FreeRTOS Task Notification
      |
      v
Load SIMP Machine Code
      |
      v
Custom SIMP CPU
Fetch -> Decode -> Execute
      |
      v
SIMP Monitor I/O Registers
      |
      v
Virtual 256 x 256 Monitor
      |
      v
Framebuffer Conversion
4 x 4 SIMP pixels -> 1 OLED pixel
      |
      v
64 x 64 Image
      |
      v
Centered on 128 x 64 SSD1306
      |
      v
Physical OLED Display
      |
      v
Buzzer Notification


<img width="1280" height="1172" alt="photo_5859308275310989042_y" src="https://github.com/user-attachments/assets/2e0e6353-7be2-4e4e-b955-5f3c9a568ca7" />

