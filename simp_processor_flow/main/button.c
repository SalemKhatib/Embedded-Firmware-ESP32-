#include "driver/gpio.h"          // ESP-IDF GPIO functions/constants
#include "esp_err.h"              // ESP_ERROR_CHECK()
#include "esp_attr.h"             // IRAM_ATTR

#include "freertos/FreeRTOS.h"    // Main FreeRTOS definitions
#include "freertos/task.h"        // Task functions/types:
                                  // TaskHandle_t, vTaskDelay(),
                                  // xTaskGetCurrentTaskHandle(), etc.

#include "button.h"


/*
 * OUR readable name for GPIO4.
 *
 * GPIO_NUM_4 is an ESP-IDF constant representing pin 4.
 */
#define BUTTON_PIN GPIO_NUM_4


/*
 * TaskHandle_t:
 *     FreeRTOS type used to store a reference/identity of a task.
 *
 * button_task:
 *     OUR variable name.
 *
 * static:
 *     Keeps this variable private to button.c.
 *
 * NULL:
 *     At startup it does not refer to any task yet.
 *
 * Later, button_init() stores the task running app_main() here.
 * The ISR then knows WHICH task it should notify.
 */
static TaskHandle_t button_task = NULL;


/*
 * ISR = Interrupt Service Routine.
 *
 * This function runs automatically when the configured
 * button interrupt occurs.
 *
 * IRAM_ATTR is an ESP-IDF attribute used for ISR code.
 *
 * IMPORTANT:
 * Keep an ISR short.
 *
 * We do NOT debounce, wait, beep, or do long work here.
 * The ISR only tells a normal FreeRTOS task:
 *
 *          "A button event happened."
 */
static void IRAM_ATTR button_isr_handler(void *arg)
{
    /*
     * BaseType_t:
     *     FreeRTOS integer-like type commonly used for
     *     status/true/false information.
     *
     * higher_priority_task_woken:
     *     OUR variable.
     *
     * Start with pdFALSE:
     *     assume the notification does NOT require an
     *     immediate task switch.
     *
     * FreeRTOS may change this variable for us below.
     */
    BaseType_t higher_priority_task_woken = pdFALSE;


    /*
     * Send a FreeRTOS task notification FROM AN ISR.
     *
     * button_task
     *     = WHICH task should receive the notification.
     *
     * &higher_priority_task_woken
     *     = address of our variable, so FreeRTOS can
     *       write back whether waking the task requires
     *       an immediate scheduler switch.
     *
     * "FromISR" is important:
     *     we are currently inside interrupt context,
     *     not normal task code.
     */
    vTaskNotifyGiveFromISR(
        button_task,
        &higher_priority_task_woken
    );


    /*
     * If the ISR woke a task that should run immediately,
     * ask FreeRTOS to perform the required scheduling
     * switch as the ISR finishes.
     */
    if (higher_priority_task_woken == pdTRUE)
    {
        portYIELD_FROM_ISR();
    }
}


/*
 * Configure the button hardware and its interrupt.
 *
 * This function is called from app_main().
 */
void button_init(void)
{
    /*
     * Ask FreeRTOS:
     *
     *     "Which task is executing this code right now?"
     *
     * Because button_init() is called from app_main(),
     * this stores the handle of the task running app_main().
     *
     * Later:
     *
     * button interrupt
     *        |
     *        v
     * ISR uses button_task
     *        |
     *        v
     * app_main's task is notified
     */
    button_task = xTaskGetCurrentTaskHandle();


    /*
     * Reset GPIO4 to a clean/default configuration
     * before configuring it for our button.
     */
    gpio_reset_pin(BUTTON_PIN);


    /*
     * The button is an INPUT:
     *
     * ESP32 <---- button
     */
    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);


    /*
     * Enable the ESP32's internal pull-up resistor.
     *
     * This gives us ACTIVE-LOW behavior:
     *
     * RELEASED = HIGH = 1
     * PRESSED  = LOW  = 0
     */
    gpio_set_pull_mode(BUTTON_PIN, GPIO_PULLUP_ONLY);


    /*
     * Generate an interrupt on a NEGATIVE/FALLING edge.
     *
     * Button press:
     *
     *      HIGH --------+
     *                   |
     *                   +------ LOW
     *
     *             1 ---> 0
     *
     * That is why GPIO_INTR_NEGEDGE is used.
     */
    gpio_set_intr_type(BUTTON_PIN, GPIO_INTR_NEGEDGE);


    /*
     * Install ESP-IDF's GPIO ISR service.
     *
     * This infrastructure allows GPIO interrupts
     * to call handlers that we register.
     *
     * ESP_ERROR_CHECK:
     * if the ESP-IDF function reports an error,
     * stop and report it instead of silently continuing.
     */
    ESP_ERROR_CHECK(gpio_install_isr_service(0));


    /*
     * Connect OUR interrupt handler to BUTTON_PIN.
     *
     * BUTTON_PIN
     *      = GPIO whose interrupt we care about.
     *
     * button_isr_handler
     *      = function to call when it happens.
     *
     * NULL
     *      = we are not passing any custom argument
     *        to the ISR.
     */
    ESP_ERROR_CHECK(
        gpio_isr_handler_add(
            BUTTON_PIN,
            button_isr_handler,
            NULL
        )
    );
}


/*
 * Check whether this was a real/stable button press.
 *
 * Mechanical buttons "bounce":
 *
 * Instead of instantly becoming:
 *
 *      1 ---------- 0
 *
 * the electrical signal may briefly look more like:
 *
 *      1 ---0-1-0-1--- 0
 *
 * So we read twice with a 20 ms delay between readings.
 */
int button_is_pressed(void)
{
    /*
     * Read GPIO4.
     *
     * Because the button is active-low:
     *
     * 0 = pressed
     * 1 = released
     */
    int first_read = gpio_get_level(BUTTON_PIN);


    /*
     * Block THIS FreeRTOS task for about 20 ms.
     *
     * This does NOT freeze the CPU.
     * Other READY tasks may run while this task waits.
     *
     * pdMS_TO_TICKS(20):
     * converts our human-friendly 20 milliseconds
     * into the RTOS tick units expected by vTaskDelay().
     */
    vTaskDelay(pdMS_TO_TICKS(20));


    /*
     * Read the button again after the debounce delay.
     */
    int second_read = gpio_get_level(BUTTON_PIN);


    /*
     * Only accept the press if BOTH readings were LOW.
     *
     * first_read  == 0
     * AND
     * second_read == 0
     *
     *          |
     *          v
     *
     * stable press
     */
    return (first_read == 0 && second_read == 0);
}


/*
 * Do not continue until the user has actually
 * released the button.
 *
 * Because released = HIGH, we wait until we get
 * two stable HIGH readings.
 */
void button_wait_for_release(void)
{
    while (1)
    {
        int first_read = gpio_get_level(BUTTON_PIN);

        /*
         * Again, this blocks only the current task.
         * Other FreeRTOS tasks can still use the CPU.
         */
        vTaskDelay(pdMS_TO_TICKS(20));

        int second_read = gpio_get_level(BUTTON_PIN);


        /*
         * Two HIGH readings means the button appears
         * to be stably released.
         */
        if (first_read == 1 && second_read == 1)
        {
            /*
             * Leave this function and return to
             * whoever called button_wait_for_release().
             */
            return;
        }
    }
}