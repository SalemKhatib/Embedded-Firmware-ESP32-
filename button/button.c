#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_attr.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "button.h"

#define BUTTON_PIN GPIO_NUM_4


/*
 * [OURS]
 * Stores the FreeRTOS task that should be notified
 * when the button interrupt occurs.
 */
static TaskHandle_t button_task = NULL;


/*
 * [OURS] Interrupt Service Routine
 */
static void IRAM_ATTR button_isr_handler(void *arg)
{
    BaseType_t higher_priority_task_woken = pdFALSE;

    /*
     * [FreeRTOS]
     * Send a notification directly to the task.
     */
    vTaskNotifyGiveFromISR(
        button_task,
        &higher_priority_task_woken
    );

    /*
     * If waking the task requires an immediate
     * scheduler switch, request one.
     */
    if (higher_priority_task_woken == pdTRUE)
    {
        portYIELD_FROM_ISR();
    }
}


void button_init(void)
{
    /*
     * [FreeRTOS]
     * Save the handle of the task currently running.
     *
     * button_init() is called from app_main(),
     * so this is the task running app_main().
     */
    button_task = xTaskGetCurrentTaskHandle();

    gpio_reset_pin(BUTTON_PIN);

    gpio_set_direction(
        BUTTON_PIN,
        GPIO_MODE_INPUT
    );

    gpio_set_pull_mode(
        BUTTON_PIN,
        GPIO_PULLUP_ONLY
    );

    /*
     * Button press:
     *
     * HIGH -> LOW
     */
    gpio_set_intr_type(
        BUTTON_PIN,
        GPIO_INTR_NEGEDGE
    );

    ESP_ERROR_CHECK(
        gpio_install_isr_service(0)
    );

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

    return (first_read == 0 &&
            second_read == 0);
}


void button_wait_for_release(void)
{
    while (1)
    {
        int first_read = gpio_get_level(BUTTON_PIN);

        vTaskDelay(pdMS_TO_TICKS(20));

        int second_read = gpio_get_level(BUTTON_PIN);

        if (first_read == 1 &&
            second_read == 1)
        {
            return;
        }
    }
}
