#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "oled.h"
#include "button.h"
#include "buzzer.h"


#define SIMP_MONITOR_WIDTH   256
#define SIMP_MONITOR_HEIGHT  256


/*
 * [OURS]
 *
 * Temporary fake SIMP monitor used while learning the display bridge.
 * The real SIMP processor will eventually write into a monitor array
 * with the same 256 x 256 layout.
 */
static uint8_t test_monitor[
    SIMP_MONITOR_WIDTH * SIMP_MONITOR_HEIGHT
];


/*
 * [OURS]
 *
 * Second FreeRTOS task used to prove that app_main can block while
 * another READY task continues running.
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
 * [OURS]
 *
 * Create a simple fake SIMP image: a diagonal line.
 *
 * Pixel (x, y) in a 1D 256-wide array lives at:
 *
 *     index = y * 256 + x
 *
 * Here x == y, so the diagonal pixels are:
 * (0,0), (1,1), (2,2), ... (255,255).
 */
static void create_test_image(void)
{
    memset(
        test_monitor,
        0,
        sizeof(test_monitor)
    );

    for (int i = 0; i < SIMP_MONITOR_WIDTH; i++)
    {
        test_monitor[
            i * SIMP_MONITOR_WIDTH + i
        ] = 0xFF;
    }
}


/*
 * [ESP-IDF]
 *
 * ESP-IDF calls app_main() for us.
 */
void app_main(void)
{
    button_init();
    buzzer_init();
    oled_init();

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
         * Sleep until the button ISR sends this task a notification.
         */
        ulTaskNotifyTake(
            pdTRUE,
            portMAX_DELAY
        );

        if (button_is_pressed())
        {
            printf("Valid button press! Showing fake SIMP monitor.\n");

            /*
             * Stage 1 of the SIMP project:
             *
             * fake SIMP monitor
             *      -> OLED conversion
             *      -> physical SSD1306
             */
            create_test_image();
            oled_show_simp_monitor(test_monitor);

            buzzer_on();
            vTaskDelay(pdMS_TO_TICKS(200));
            buzzer_off();

            /* Keep the image visible while the button is held. */
            button_wait_for_release();

            oled_clear();
        }

        /* Discard notifications caused by switch bounce. */
        ulTaskNotifyTake(
            pdTRUE,
            0
        );
    }
}
