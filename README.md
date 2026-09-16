# Embedded Firmware ESP32

This repository documents two connected parts of my ESP32 firmware learning work:

1. **`learning_basics/`** - the incremental exercises used to learn GPIO, button debouncing, interrupts, FreeRTOS task notifications, a buzzer, I2C, and SSD1306 OLED control.
2. **`simp_processor_flow/`** - the active integration project: reuse a SIMP CPU simulator written in C and eventually display SIMP graphics programs on a physical OLED connected to the ESP32.

## Repository Layout

```text
Embedded-Firmware-ESP32/
|
|-- learning_basics/
|   `-- original incremental ESP32 learning project
|
`-- simp_processor_flow/
    |-- main/
    |-- components/
    |-- docs/
    `-- reference/
```

## Current SIMP Project Status

The processor is not connected yet. The current active milestone is deliberately smaller:

```text
fake 256 x 256 SIMP monitor
        |
        v
OLED conversion code
        |
        v
physical 128 x 64 SSD1306
```

Once that bridge is understood and tested, the stripped SIMP CPU core will be integrated so that the original `circle` program can generate the virtual monitor contents itself.

The longer-term goal is that new SIMP assembly programs such as stairs, rectangles, or triangles can run through the same processor and display path without shape-specific ESP32 code.

## Why Keep the Basics Folder?

The basics project shows the progression that led to the SIMP integration work instead of presenting only the final result. It includes the steps from GPIO polling through interrupts, FreeRTOS synchronization, and OLED control.

For the active work, see [`simp_processor_flow/`](simp_processor_flow/).
