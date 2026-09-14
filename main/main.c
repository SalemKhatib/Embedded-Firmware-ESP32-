#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "oled.h"
#include "button.h"
#include "buzzer.h"


/*
 * OUR second FreeRTOS task.
 *
 * Prints once every second.
 */
void background_task(void *arg)
{
    while (1)
    {
        printf("Background task is running...\n");

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


/*
 * ESP-IDF calls app_main() for us.
 */
void app_main(void)
{
    /*
     * Initialize hardware.
     */
    button_init();
    buzzer_init();
    oled_init();


    /*
     * oled_init() clears the screen,
     * so the OLED starts BLACK.
     */


    /*
     * Create the background FreeRTOS task.
     */
    xTaskCreate(
        background_task,
        "background_task",
        2048,
        NULL,
        1,
        NULL
    );


    while (1)
    {
        /*
         * Sleep until the button ISR
         * sends this task a notification.
         */
        ulTaskNotifyTake(
            pdTRUE,
            portMAX_DELAY
        );


        /*
         * Verify/debounce the button press.
         */
        if (button_is_pressed())
        {
            printf("Valid button press!\n");


            /*
             * BUTTON PRESSED:
             * make entire OLED white.
             */
            oled_fill(1);


            /*
             * Beep for 200 ms.
             */
            buzzer_on();

            vTaskDelay(pdMS_TO_TICKS(200));

            buzzer_off();


            /*
             * Stay here until the button
             * has been released.
             *
             * OLED remains white while held.
             */
            button_wait_for_release();


            /*
             * BUTTON RELEASED:
             * make OLED black again.
             */
            oled_clear();
        }


        /*
         * Discard leftover notifications
         * caused by button bounce.
         */
        ulTaskNotifyTake(
            pdTRUE,
            0
        );
    }
}
