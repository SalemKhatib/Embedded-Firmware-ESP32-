#include <stdio.h>                // printf()

#include "freertos/FreeRTOS.h"    // Core FreeRTOS definitions
#include "freertos/task.h"        // Task APIs such as:
                                  // xTaskCreate(),
                                  // vTaskDelay(),
                                  // ulTaskNotifyTake()

#include "button.h"
#include "buzzer.h"


/*
 * OUR function that will become the code executed
 * by a second FreeRTOS task.
 *
 * IMPORTANT:
 *
 * Writing a C function does NOT automatically create a task.
 *
 *      background_task()
 *              =
 *         just a C function
 *
 * Later, xTaskCreate() asks FreeRTOS to create a task
 * that starts executing this function.
 *
 * void *arg is the optional argument FreeRTOS can pass
 * into a task function. We do not use it in this example.
 */
void background_task(void *arg)
{
    /*
     * FreeRTOS tasks commonly contain an infinite loop,
     * because the task is intended to keep doing its job
     * for as long as the system is running.
     */
    while (1)
    {
        printf("Background task is running...\n");


        /*
         * Block THIS task for about 1000 ms = 1 second.
         *
         * The CPU is NOT frozen.
         *
         * While background_task is BLOCKED,
         * FreeRTOS may schedule another READY task.
         */
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


/*
 * ESP-IDF calls app_main() for us.
 *
 * Important FreeRTOS idea:
 *
 * app_main() is ALREADY executing inside an
 * ESP-IDF/FreeRTOS task.
 *
 * We do not need xTaskCreate() for app_main itself.
 */
void app_main(void)
{
    /*
     * Configure the button.
     *
     * Inside button_init(), we also save:
     *
     *     xTaskGetCurrentTaskHandle()
     *
     * so button.c remembers the handle of THIS task.
     *
     * The GPIO ISR will later notify this task.
     */
    button_init();


    /*
     * Configure the buzzer hardware.
     */
    buzzer_init();


    /*
     * Create ANOTHER FreeRTOS task.
     *
     * xTaskCreate() does not simply call
     * background_task() like a normal C function.
     *
     * It asks the FreeRTOS scheduler to create
     * another independently scheduled task.
     */
    xTaskCreate(

        /*
         * 1) Function the new task should execute.
         */
        background_task,

        /*
         * 2) Human-readable task name.
         *
         * Useful for debugging/task information.
         */
        "background_task",

        /*
         * 3) Stack size allocated for this task.
         */
        2048,

        /*
         * 4) Argument passed to background_task(void *arg).
         *
         * NULL = we do not need to pass anything.
         */
        NULL,

        /*
         * 5) Task priority.
         *
         * FreeRTOS uses priorities when deciding which
         * READY task should run.
         */
        1,

        /*
         * 6) Optional place to store the new task's handle.
         *
         * We never need to directly identify
         * background_task later, so we do not save one.
         */
        NULL
    );


    /*
     * app_main's task now spends its life waiting
     * for and processing button events.
     */
    while (1)
    {
        /*
         * WAIT FOR A BUTTON NOTIFICATION.
         *
         * This pairs with:
         *
         * button.c:
         *
         *     vTaskNotifyGiveFromISR(...)
         *
         *
         * Think:
         *
         * ISR                                app_main
         *
         * Notify --------------------------> Take
         *
         *
         * If no notification exists,
         * app_main's task becomes BLOCKED.
         *
         * BLOCKED TASK != BLOCKED CPU.
         *
         * FreeRTOS can run background_task while
         * app_main waits here.
         */
        ulTaskNotifyTake(

            /*
             * pdTRUE:
             *
             * when we successfully take the notification,
             * clear its pending notification count to 0.
             */
            pdTRUE,

            /*
             * portMAX_DELAY:
             *
             * Wait indefinitely if necessary.
             *
             * In other words:
             *
             * "I have nothing useful to do until
             *  somebody notifies me."
             */
            portMAX_DELAY
        );


        /*
         * We only reach this line AFTER a notification
         * has woken this task.
         *
         * The interrupt tells us:
         *
         *     "Something happened on the button."
         *
         * We still debounce/check it here to confirm
         * that the button is really pressed.
         */
        if (button_is_pressed())
        {
            printf("Valid button press!\n");


            /*
             * Turn the active buzzer on.
             */
            buzzer_on();


            /*
             * Keep the buzzer on for about 200 ms.
             *
             * Again, vTaskDelay() blocks THIS task,
             * not the whole processor.
             */
            vTaskDelay(pdMS_TO_TICKS(200));


            /*
             * Turn the buzzer back off.
             */
            buzzer_off();


            /*
             * Wait until the same physical button
             * press has been released.
             */
            button_wait_for_release();
        }


        /*
         * There may have been extra interrupts from
         * mechanical button bounce while we were
         * handling the same physical press.
         *
         * So perform another notification take.
         *
         * pdTRUE:
         *     clear the pending notification count.
         *
         * 0:
         *     DO NOT wait if nothing is pending.
         *
         * Compare:
         *
         * portMAX_DELAY -> "wait until one arrives"
         *
         * 0             -> "check/take now; never wait"
         *
         * This helps discard extra notifications
         * left over from the same bouncing press.
         */
        ulTaskNotifyTake(
            pdTRUE,
            0
        );
    }
}
