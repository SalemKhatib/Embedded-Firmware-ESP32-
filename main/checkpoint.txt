## Current Checkpoint

At this stage, the project supports:

- GPIO output using an active buzzer
- GPIO input using a pushbutton
- ESP32 internal pull-up resistor
- Active-low button logic
- FreeRTOS delays with `vTaskDelay()`
- Basic modular C structure using `.c` and `.h` files
- Combining an input with an output

Current behavior:

```text
Button released
    ↓
GPIO4 reads HIGH
    ↓
Buzzer OFF

Button pressed
    ↓
GPIO4 reads LOW
    ↓
Buzzer ON
