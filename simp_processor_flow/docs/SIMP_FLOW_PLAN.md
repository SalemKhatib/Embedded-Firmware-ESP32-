# SIMP-on-ESP32 Learning Plan

The project is intentionally being built in layers.

## Stage 1 - Display Bridge (current)

Create a fake 256x256 SIMP-style monitor in RAM and convert it into the physical SSD1306 framebuffer.

Success condition: a known pattern such as a diagonal line appears correctly on the OLED.

## Stage 2 - Understand the Mapping

Be able to explain both representations:

```text
SIMP monitor:
monitor[y * 256 + x]

OLED framebuffer:
framebuffer[page * 128 + x]
page = y / 8
bit  = y % 8
```

## Stage 3 - Add the Trimmed SIMP Core

Bring in the reusable pieces from the previous Computer Structure simulator:

- memory[4096]
- registers[16]
- PC
- instruction decoder
- immediate handling
- arithmetic / logical instructions
- branches
- load/store
- IN / OUT
- HALT
- virtual monitor registers

Disk, timer, external IRQ input files, trace files, and course checker output files stay out of V1.

## Stage 4 - Load a Program Without Desktop Files

Replace `fopen(memin.txt)` with a machine-code array compiled into firmware and copy that array into SIMP memory.

## Stage 5 - Run `circle`

Run the original circle machine code until HALT and verify that the SIMP virtual monitor contains the expected image.

## Stage 6 - Connect CPU Output to OLED

Reuse the exact Stage 1 display bridge:

```text
circle program
    -> SIMP CPU
    -> monitor[]
    -> oled_show_simp_monitor()
    -> OLED
```

## Stage 7 - New Graphics Programs

Write new SIMP assembly programs without changing the OLED code, for example:

- diagonal line
- stairs
- rectangle
- triangle

The ESP32 should know only how to display the SIMP monitor; it should not contain shape-specific drawing hacks.
