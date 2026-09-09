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
    return gpio_get_level(BUTTON_PIN) == 0;
}