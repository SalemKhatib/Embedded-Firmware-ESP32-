#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "buzzer.h"

#define BUZZER_PIN GPIO_NUM_23

void buzzer_demo(void)
{
    gpio_reset_pin(BUZZER_PIN);
    gpio_set_direction(BUZZER_PIN, GPIO_MODE_OUTPUT);

    while (1)
    {
        gpio_set_level(BUZZER_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(500));

        gpio_set_level(BUZZER_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
