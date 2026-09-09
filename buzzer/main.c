#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define BUZZER_PIN GPIO_NUM_23

void app_main(void)
{
    /* Reset GPIO23 to its default configuration */
    gpio_reset_pin(BUZZER_PIN);

    /* Configure GPIO23 as an output */
    gpio_set_direction(BUZZER_PIN, GPIO_MODE_OUTPUT);

    while (1)
    {
        /* Turn active buzzer ON */
        gpio_set_level(BUZZER_PIN, 1);

        /* Keep it on for 500 ms */
        vTaskDelay(pdMS_TO_TICKS(500));

        /* Turn buzzer OFF */
        gpio_set_level(BUZZER_PIN, 0);

        /* Keep it off for 500 ms */
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}