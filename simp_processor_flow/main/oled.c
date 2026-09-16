#include <string.h>

#include "driver/gpio.h"
#include "driver/i2c_master.h"

#include "esp_err.h"

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_ssd1306.h"

#include "oled.h"


#define OLED_SDA       GPIO_NUM_21
#define OLED_SCL       GPIO_NUM_22
#define OLED_ADDRESS   0x3C
#define SIMP_MONITOR_WIDTH   256
#define SIMP_MONITOR_HEIGHT  256

#define OLED_WIDTH     128
#define OLED_HEIGHT    64


/*
 * [ESP-IDF type]
 *
 * This handle identifies our SSD1306 panel after
 * ESP-IDF creates it.
 */
static esp_lcd_panel_handle_t oled_panel = NULL;


/*
 * [OURS]
 *
 * 128 x 64 pixels
 * 1 bit per pixel
 *
 * 128 * 64 / 8 = 1024 bytes
 */
static uint8_t framebuffer[OLED_WIDTH * OLED_HEIGHT / 8];
static void oled_set_pixel(int x, int y)
{
    int page = y / 8;
    int bit  = y % 8;

    framebuffer[page * OLED_WIDTH + x] |= (1 << bit);
}

void oled_show_simp_monitor(const uint8_t *simp_monitor)
{
    /*
     * Start with a completely black OLED framebuffer.
     */
    memset(framebuffer, 0x00, sizeof(framebuffer));


    /*
     * The SIMP monitor is 256 x 256.
     *
     * We convert each 4 x 4 SIMP block
     * into one OLED pixel.
     */
    for (int oled_y = 0; oled_y < 64; oled_y++)
    {
        for (int oled_x = 0; oled_x < 64; oled_x++)
        {
            int pixel_on = 0;


            /*
             * Look at one 4 x 4 block
             * of the SIMP monitor.
             */
            for (int dy = 0; dy < 4; dy++)
            {
                for (int dx = 0; dx < 4; dx++)
                {
                    int simp_x = oled_x * 4 + dx;
                    int simp_y = oled_y * 4 + dy;

                    int simp_index =
                        simp_y * SIMP_MONITOR_WIDTH
                        + simp_x;


                    /*
                     * Any non-zero SIMP pixel
                     * becomes white on the OLED.
                     */
                    if (simp_monitor[simp_index] != 0)
                    {
                        pixel_on = 1;
                    }
                }
            }


            /*
             * The converted image is only 64 pixels wide.
             *
             * Put 32 black pixels on the left,
             * so it is centered on the 128-pixel OLED.
             */
            if (pixel_on)
            {
                oled_set_pixel(
                    oled_x + 32,
                    oled_y
                );
            }
        }
    }


    /*
     * Send our completed framebuffer
     * to the physical OLED.
     */
    ESP_ERROR_CHECK(
        esp_lcd_panel_draw_bitmap(
            oled_panel,
            0,
            0,
            OLED_WIDTH,
            OLED_HEIGHT,
            framebuffer
        )
    );
}

void oled_init(void)
{
    /*
     * -----------------------------
     * STEP 1: Create the I2C bus
     * -----------------------------
     */

    i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,

        .sda_io_num = OLED_SDA,
        .scl_io_num = OLED_SCL,

        .glitch_ignore_cnt = 7,

        .flags.enable_internal_pullup = true,
    };


    i2c_master_bus_handle_t i2c_bus = NULL;

    ESP_ERROR_CHECK(
        i2c_new_master_bus(
            &bus_config,
            &i2c_bus
        )
    );


    /*
     * ---------------------------------
     * STEP 2: Connect the OLED to I2C
     * ---------------------------------
     */

    esp_lcd_panel_io_i2c_config_t io_config = {
        .dev_addr = OLED_ADDRESS,

        .scl_speed_hz = 400000,

        .control_phase_bytes = 1,

        .dc_bit_offset = 6,

        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
    };


    esp_lcd_panel_io_handle_t io_handle = NULL;

    ESP_ERROR_CHECK(
        esp_lcd_new_panel_io_i2c(
            i2c_bus,
            &io_config,
            &io_handle
        )
    );


    /*
     * ---------------------------------
     * STEP 3: Create SSD1306 panel
     * ---------------------------------
     */

    esp_lcd_panel_ssd1306_config_t ssd1306_config = {
        .height = OLED_HEIGHT,
    };


    esp_lcd_panel_dev_config_t panel_config = {
        .bits_per_pixel = 1,
        .reset_gpio_num = -1,

        .vendor_config = &ssd1306_config,
    };


    ESP_ERROR_CHECK(
        esp_lcd_new_panel_ssd1306(
            io_handle,
            &panel_config,
            &oled_panel
        )
    );


    /*
     * ---------------------------------
     * STEP 4: Actually initialize OLED
     * ---------------------------------
     */

    ESP_ERROR_CHECK(
        esp_lcd_panel_reset(oled_panel)
    );

    ESP_ERROR_CHECK(
        esp_lcd_panel_init(oled_panel)
    );


    /*
     * Our particular OLED orientation.
     */
    ESP_ERROR_CHECK(
        esp_lcd_panel_mirror(
            oled_panel,
            true,
            true
        )
    );


    /*
     * Turn the physical display on.
     */
    ESP_ERROR_CHECK(
        esp_lcd_panel_disp_on_off(
            oled_panel,
            true
        )
    );


    /*
     * Start with a blank screen.
     */
    oled_clear();
}


void oled_fill(int on)
{
    /*
     * 0xFF = 11111111
     *        all eight bits ON
     *
     * 0x00 = 00000000
     *        all eight bits OFF
     */

    if (on)
    {
        memset(
            framebuffer,
            0xFF,
            sizeof(framebuffer)
        );
    }
    else
    {
        memset(
            framebuffer,
            0x00,
            sizeof(framebuffer)
        );
    }


    /*
     * Send the framebuffer to the OLED.
     */
    ESP_ERROR_CHECK(
        esp_lcd_panel_draw_bitmap(
            oled_panel,
            0,
            0,
            OLED_WIDTH,
            OLED_HEIGHT,
            framebuffer
        )
    );
}


void oled_clear(void)
{
    oled_fill(0);
}