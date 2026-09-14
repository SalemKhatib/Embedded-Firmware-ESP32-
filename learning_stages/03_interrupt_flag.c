#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "button.h"
#include "buzzer.h"

void app_main(void)
{
    button_init();
    buzzer_init();

    while (1)
    {
        if (button_event_pending())
        {
            if (button_is_pressed())
            {
                printf("Valid button press!\n");

                buzzer_on();

                vTaskDelay(pdMS_TO_TICKS(200));

                buzzer_off();

                button_wait_for_release();
            }

            button_clear_event();
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
