#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "button.h"

void app_main(void)
{
    button_init();

    int count = 0;
    int previous_state = button_is_pressed();

    while (1)
    {
        int current_state = button_is_pressed();

        // Detect only a new press:
        // previous = released, current = pressed
        if (current_state == 1 && previous_state == 0)
        {
            count++;

            printf("Press count = %d\n", count);
        }

        previous_state = current_state;

        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
