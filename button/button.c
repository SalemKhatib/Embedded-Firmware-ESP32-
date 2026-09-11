#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_attr.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "button.h"

#define BUTTON_PIN GPIO_NUM_4

static volatile int button_event = 0;

/* Runs automatically when GPIO4 detects HIGH -> LOW */
static void IRAM_ATTR button_isr_handler(void *arg)
{
    button_event = 1;
}

void button_init(void)
{
    gpio_reset_pin(BUTTON_PIN);

    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);

    gpio_set_pull_mode(BUTTON_PIN, GPIO_PULLUP_ONLY);

    /* Pressing the button gives HIGH -> LOW */
    gpio_set_intr_type(BUTTON_PIN, GPIO_INTR_NEGEDGE);

    /* Start ESP-IDF GPIO interrupt service */
    ESP_ERROR_CHECK(
        gpio_install_isr_service(0)
    );

    /* Connect GPIO4 interrupt to our ISR */
    ESP_ERROR_CHECK(
        gpio_isr_handler_add(
            BUTTON_PIN,
            button_isr_handler,
            NULL
        )
    );
}

int button_is_pressed(void)
{
    int first_read = gpio_get_level(BUTTON_PIN);

    vTaskDelay(pdMS_TO_TICKS(20));

    int second_read = gpio_get_level(BUTTON_PIN);

    return (first_read == 0 && second_read == 0);
}

int button_event_pending(void)
{
    return button_event;
}

void button_clear_event(void)
{
    button_event = 0;
}

void button_wait_for_release(void)
{
    while (1)
    {
        int first_read = gpio_get_level(BUTTON_PIN);

        vTaskDelay(pdMS_TO_TICKS(20));

        int second_read = gpio_get_level(BUTTON_PIN);

        if (first_read == 1 && second_read == 1)
        {
            return;
        }
    }
}
