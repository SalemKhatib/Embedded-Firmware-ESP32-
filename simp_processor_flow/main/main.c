#include <stdio.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "simp.h"
#include "oled.h"
#include "button.h"
#include "buzzer.h"


/*
 * [OURS]
 *
 * Our second FreeRTOS task.
 *
 * Prints once every second.
 */
void background_task(void *arg)
{
    while (1)
    {
        printf("Background task is running...\n");

        vTaskDelay(
            pdMS_TO_TICKS(1000)
        );
    }
}


/*
 * [OURS]
 *
 * The SIMP program currently being tested.
 *
 * Right now this contains the assembled
 * machine code for circle.asm.
 *
 * Later we can replace the contents with
 * stairs, triangle, square, etc.
 */
static const uint32_t simp_test_program[] =
{
    0x00810040u,   // y = 64
    0x00D100C0u,   // end_y = 192
    0x006100FFu,   // white = 255
    0x00710001u,   // monitor command = 1

    /*
     * row_loop:
     */
    0x01981040u,   // half_width = y - 64
    0x01B19080u,   // left  = 128 - half_width
    0x00C19080u,   // right = 128 + half_width
    0x07481008u,   // row_base = y << 8
    0x00AB0000u,   // x = left

    /*
     * pixel_loop:
     */
    0x0054A000u,   // pixel address = row_base + x

    0x15510014u,   // monitoraddr = pixel address
    0x15610015u,   // monitordata = 255
    0x15710016u,   // monitorcmd = 1

    0x00A1A001u,   // x++

    0x0E1AC009u,   // if x <= right, go to pixel_loop

    0x00818001u,   // y++

    0x0C18D004u,   // if y < 192, go to row_loop

    0x16000000u    // halt
};


/*
 * [ESP-IDF]
 *
 * ESP-IDF calls app_main() for us.
 */
void app_main(void)
{
    /*
     * Initialize physical hardware.
     */
    button_init();
    buzzer_init();
    oled_init();


    /*
     * Create our second FreeRTOS task.
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
         * [FreeRTOS]
         *
         * Sleep this task until the
         * button interrupt wakes us.
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
             * Clear the physical OLED
             * before starting a new run.
             */
            oled_clear();


            /*
             * Reset the virtual SIMP machine.
             *
             * Clears:
             *
             * memory
             * registers
             * I/O registers
             * monitor
             * PC
             * halted state
             */
            simp_reset();


            /*
             * Copy simp_test_program[]
             * into the simulated SIMP memory.
             */
            simp_load_program(
                simp_test_program,
                sizeof(simp_test_program)
                    / sizeof(simp_test_program[0])
            );


            /*
             * Run the SIMP processor.
             *
             * Stop if:
             *
             * 1. SIMP executes HALT
             *
             * OR
             *
             * 2. We reach 1,000,000 cycles.
             */
            int result = simp_run(
                1000000
            );


            /*
             * Print the result of the
             * simulated processor run.
             */
            printf(
                "SIMP result: halted=%d, cycles=%lu\n",
                result,
                (unsigned long)simp_get_cycles()
            );


            /*
             * Get the REAL monitor[]
             * produced by the SIMP program
             * and show it on the OLED.
             */
            oled_show_simp_monitor(
                simp_get_monitor()
            );


            /*
             * Beep when the SIMP program
             * has finished running.
             */
            buzzer_on();

            vTaskDelay(
                pdMS_TO_TICKS(200)
            );

            buzzer_off();


            /*
             * Wait until the physical
             * button is released.
             */
            button_wait_for_release();


            /*
             * We DO NOT call oled_clear()
             * here.
             *
             * The resulting SIMP image
             * stays visible on the OLED.
             */
        }


        /*
         * Discard any leftover notifications
         * caused by button bounce.
         */
        ulTaskNotifyTake(
            pdTRUE,
            0
        );
    }
}