# Original Simulator: Keep / Remove / Change Map

This document records the first review of the original ~580-line Computer Structure simulator before ESP32 integration.

## Keep

These are part of the SIMP machine or useful to the first graphics version:

- `MEM_SIZE`, `NUM_REGS`, `IOREG_SIZE`, `PC_MASK`
- `MONITOR_WIDTH`, `MONITOR_HEIGHT`
- opcode enum
- `memory[]`
- `registers[]`
- `IORegister[]`
- `monitor[]`
- `PC`
- `halted`
- `first_cycle_imm2`
- instruction field macros
- immediate sign extension
- `$imm2` two-word timing
- `mask_ioreg_write()`
- `out_opcode()` monitor behavior
- `fetch_decode_execute()`
- arithmetic / logical instructions
- branches
- `jal`
- `lw`, `sw`
- `in`, `out`
- `halt`

## Remove for V1

These belong to the desktop/course test environment or peripherals not needed for the first graphics goal:

- `_CRT_SECURE_NO_WARNINGS`
- disk array and disk controller timing
- disk input/output files
- IRQ2 input file parsing
- timer update logic
- IRQ acceptance logic
- dynamic IRQ2 allocation
- trace file output
- hardware-register trace output
- `memout.txt`
- `regout.txt`
- `cycles.txt`
- `monitor.txt`
- `monitor.yuv` file output
- LED / seven-segment file logging
- desktop `main(argc, argv)`

## Change

- `load_input_files()` -> later becomes an embedded `load_program()` that copies a compiled machine-code array into `memory[]`.
- desktop `main()` simulation loop -> later becomes a reusable SIMP run function called from ESP-IDF `app_main()`.
- monitor output files -> physical OLED rendering.
