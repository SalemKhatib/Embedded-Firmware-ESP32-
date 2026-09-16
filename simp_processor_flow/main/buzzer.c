#include "driver/gpio.h"
#include "buzzer.h"

#define BUZZER_PIN GPIO_NUM_23

void buzzer_init(void)
{
    gpio_reset_pin(BUZZER_PIN);
    gpio_set_direction(BUZZER_PIN, GPIO_MODE_OUTPUT);

    buzzer_off();
}

void buzzer_on(void)
{
    gpio_set_level(BUZZER_PIN, 1);
}

void buzzer_off(void)
{
    gpio_set_level(BUZZER_PIN, 0);
}