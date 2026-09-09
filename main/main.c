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
        if (button_is_pressed())
        {
            buzzer_on();
        }
        else
        {
            buzzer_off();
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
