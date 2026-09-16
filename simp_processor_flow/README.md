# SIMP Processor Flow on ESP32

This is the **active integration project** in the repository.

The goal is to reuse a previously written SIMP CPU simulator in C and connect it to real ESP32 hardware.

The ESP32 now executes SIMP machine code, updates the simulated 256x256 SIMP monitor through the processor's I/O registers, converts that virtual monitor to the physical OLED resolution, and displays the result on a real 128x64 SSD1306 OLED.

The project is being developed in small stages so that each part of the processor, FreeRTOS control flow, and hardware interface can be understood and tested independently.

## Current Stage

The full SIMP-to-OLED path is now working.

The current flow is:

```text
physical button press
        |
        v
FreeRTOS task notification
        |
        v
simp_reset()
        |
        v
simp_load_program()
        |
        v
SIMP machine code
        |
        v
fetch -> decode -> execute
        |
        v
OUT instructions
        |
        v
monitoraddr / monitordata / monitorcmd
        |
        v
virtual monitor[256 * 256]
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
physical SSD1306 OLED
        |
        v
buzzer notification
```

The ESP32 display code does **not** know which shape is being drawn.

It only receives the final SIMP virtual monitor and displays it.

This means the same processor and OLED path can run different SIMP programs without changing the OLED driver.

## Verified Programs

### Circle

The original SIMP `circle` program was assembled into machine code and executed by the SIMP CPU running on the ESP32.

The program:

- scans the 256x256 virtual monitor
- calculates the distance of every pixel from the center
- writes pixel values through the SIMP monitor I/O registers
- halts when finished

The resulting filled circle was successfully displayed on the physical SSD1306 OLED.

```text
circle.asm
    |
    v
assembler
    |
    v
machine code
    |
    v
SIMP CPU on ESP32
    |
    v
virtual monitor
    |
    v
OLED
```

### Triangle

A second SIMP graphics program was created to draw a filled triangle.

The program progressively increases the width of each horizontal row while writing pixels through the same SIMP monitor interface.

No ESP32 graphics code was changed.

Only the machine code stored in:

```c
simp_test_program[]
```

was replaced.

This demonstrates that the ESP32 side is acting as a generic SIMP processor/display platform rather than containing hardcoded circle-drawing logic.

A hardware test photo of the triangle will be added to the repository.

## Replaceable Test Program

For now, one array is used as the current SIMP program under test:

```c
static const uint32_t simp_test_program[]
```

The surrounding firmware does not need to change when testing another SIMP program.

For example:

```text
simp_test_program = circle
        |
        v
run processor
        |
        v
display circle
```

or:

```text
simp_test_program = triangle
        |
        v
run processor
        |
        v
display triangle
```

Future programs can include:

- lines
- stairs
- rectangles
- squares
- additional triangles
- other small graphics experiments

## Architecture

```text
                 ESP32
                   |
                   |
             FreeRTOS task
                   |
                   v
             SIMP processor
        +----------------------+
        | memory[4096]         |
        | registers[16]        |
        | PC                   |
        | I/O registers[23]    |
        | monitor[256 * 256]   |
        +----------------------+
                   |
                   |
        monitor I/O registers
        20 = monitoraddr
        21 = monitordata
        22 = monitorcmd
                   |
                   v
          OLED conversion layer
                   |
                   v
             SSD1306 OLED
```

The SIMP monitor is 256x256 pixels with one byte per pixel.

The physical OLED is 128x64.

The current display bridge reduces each 4x4 block of SIMP pixels into one OLED pixel, producing a 64x64 image which is centered horizontally on the 128x64 display.

## Current Hardware

| Device | Connection |
| --- | --- |
| Push button | GPIO4, active-low with internal pull-up |
| Active buzzer | GPIO23 |
| SSD1306 SDA | GPIO21 |
| SSD1306 SCL | GPIO22 |
| OLED address | `0x3C` |

## FreeRTOS Control Flow

The push button uses a GPIO falling-edge interrupt.

The interrupt service routine sends a FreeRTOS task notification to the main application task.

```text
button press
    |
    v
GPIO interrupt
    |
    v
ISR
    |
    v
vTaskNotifyGiveFromISR()
    |
    v
main task wakes
    |
    v
debounce / verify button
    |
    v
run SIMP program
```

A separate background FreeRTOS task is also used during development to demonstrate that other tasks continue to execute while the main task is blocked waiting for events.

## Project Structure

```text
simp_processor_flow/
|-- CMakeLists.txt
|-- README.md
|
|-- main/
|   |-- CMakeLists.txt
|   |-- main.c
|   |
|   |-- simp.c
|   |-- simp.h
|   |
|   |-- oled.c
|   |-- oled.h
|   |
|   |-- button.c
|   |-- button.h
|   |
|   |-- buzzer.c
|   `-- buzzer.h
|
|-- docs/
|   |-- SIMP_FLOW_PLAN.md
|   `-- SIMULATOR_TRIM_MAP.md
|
`-- reference/
    `-- simp_core_clean.c
```

The source files inside `main/` are the versions currently used by the ESP-IDF build.

`reference/simp_core_clean.c` remains as a historical/reference version of the simulator cleanup work.

## Main Modules

### `simp.c`

Implements the SIMP processor model.

It contains the simulated:

- memory
- register file
- program counter
- I/O registers
- 256x256 monitor
- instruction fetch/decode/execute logic

The ESP32-facing interface currently includes functions such as:

```c
simp_reset();
simp_load_program();
simp_run();
simp_get_cycles();
simp_get_monitor();
```

### `oled.c`

Controls the physical SSD1306 OLED.

It also contains the bridge between:

```text
SIMP 256x256 monitor
```

and:

```text
SSD1306 128x64 display
```

The processor does not directly know anything about the physical OLED.

### `button.c`

Handles:

- GPIO4 input
- internal pull-up
- falling-edge interrupt
- FreeRTOS task notification
- button debounce
- button release detection

### `buzzer.c`

Controls the active buzzer connected to GPIO23.

## Build

From `simp_processor_flow/` with the ESP-IDF environment loaded:

```bash
idf.py set-target esp32
idf.py build
idf.py -p COM5 flash monitor
```

## Hardware Results

The project has now physically demonstrated the complete path:

```text
button
  |
  v
SIMP machine-code program
  |
  v
custom CPU simulator
  |
  v
SIMP monitor I/O
  |
  v
virtual framebuffer
  |
  v
OLED conversion
  |
  v
real hardware display
```

Both a filled circle and a filled triangle have been successfully generated by SIMP programs and displayed on the physical OLED.

### Triangle Test

A photo of the triangle running on the physical OLED will be added here.

```markdown
![SIMP triangle running on ESP32 OLED](docs/images/triangle_oled_test.jpg)
```

## Next Steps

The next experiments will focus on:

- adding more small SIMP graphics programs
- keeping the ESP32 display code generic
- improving the organization of test programs
- continuing to understand the SIMP processor and FreeRTOS integration
- gradually expanding the project without hiding the implementation behind large abstractions
