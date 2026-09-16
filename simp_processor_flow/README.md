# SIMP Processor Flow on ESP32

This is the **active project** in the repository.

The goal is to reuse a previously written SIMP CPU simulator in C and gradually integrate it with real ESP32 hardware. The first tangible target is to run the SIMP `circle` program and display the simulated 256x256 monitor output on a physical 128x64 SSD1306 OLED.

This folder intentionally develops the integration in small, understandable stages rather than dropping in a finished emulator.

## Current Stage

The SIMP CPU is **not connected yet**.

The current build proves only the display bridge:

```text
button press
    |
    v
fake 256 x 256 SIMP monitor
    |
    v
4 x 4 virtual pixels -> 1 OLED pixel
    |
    v
64 x 64 converted image
    |
    v
center inside 128 x 64 OLED framebuffer
    |
    v
physical SSD1306
```

The fake monitor currently contains a diagonal line. This is deliberate: it lets the framebuffer conversion be understood and tested before the processor is added.

## Planned Flow

```text
SIMP assembly program
        |
        v
assembler / machine-code image
        |
        v
SIMP memory[]
        |
        v
fetch -> decode -> execute
        |
        v
OUT monitoraddr / monitordata / monitorcmd
        |
        v
virtual monitor[256 * 256]
        |
        v
OLED conversion
        |
        v
physical SSD1306
```

The first real program target is `circle`. After that, the same processor and display path should be able to run new graphics programs such as stairs, lines, rectangles, and triangles without changing the ESP32 display code.

## Current Hardware

| Device | Connection |
| --- | --- |
| Push button | GPIO4, active-low with internal pull-up |
| Active buzzer | GPIO23 |
| SSD1306 SDA | GPIO21 |
| SSD1306 SCL | GPIO22 |
| OLED address | `0x3C` |

## Structure

```text
simp_processor_flow/
|-- CMakeLists.txt
|-- README.md
|-- main/
|   |-- CMakeLists.txt
|   `-- main.c
|-- components/
|   |-- button/
|   |-- buzzer/
|   `-- oled/
|-- docs/
|   |-- SIMP_FLOW_PLAN.md
|   `-- SIMULATOR_TRIM_MAP.md
`-- reference/
    `-- simp_core_clean.c
```

`reference/simp_core_clean.c` is the stripped SIMP core we reviewed. It is intentionally **not part of the build yet**. It remains a reference until the virtual-monitor-to-OLED bridge is understood and tested.

## Build

From this folder with the ESP-IDF environment loaded:

```bash
idf.py set-target esp32
idf.py build
idf.py -p COM5 flash monitor
```

At the current stage, pressing the button should display a centered diagonal-line image generated from a fake 256x256 SIMP monitor and beep briefly.
