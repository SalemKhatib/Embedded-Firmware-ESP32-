#include "driver/gpio.h"
#include "button.h"

#define BUTTON_PIN GPIO_NUM_4

void button_init(void)
{
    gpio_reset_pin(BUTTON_PIN);

    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);

    gpio_set_pull_mode(BUTTON_PIN, GPIO_PULLUP_ONLY);
}

int button_is_pressed(void)
{
    int first_read = gpio_get_level(BUTTON_PIN);
     // Wait for mechanical bouncing to settle
    vTaskDelay(pdMS_TO_TICKS(20));
    int second_read = gpio_get_level(BUTTON_PIN);
     // Button is considered pressed only if both readings are LOW
    return (first_read == 0 && second_read == 0);
}
