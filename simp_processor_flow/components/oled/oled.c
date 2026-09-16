#include <string.h>

#include "driver/gpio.h"
#include "driver/i2c_master.h"

#include "esp_err.h"

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_ssd1306.h"

#include "oled.h"


#define OLED_SDA              GPIO_NUM_21
#define OLED_SCL              GPIO_NUM_22
#define OLED_ADDRESS          0x3C

#define OLED_WIDTH            128
#define OLED_HEIGHT           64

#define SIMP_MONITOR_WIDTH    256
#define SIMP_MONITOR_HEIGHT   256


/*
 * [ESP-IDF type]
 *
 * Handle for the SSD1306 panel created by ESP-IDF.
 */
static esp_lcd_panel_handle_t oled_panel = NULL;


/*
 * [OURS]
 *
 * Physical OLED framebuffer:
 * 128 x 64 pixels, 1 bit per pixel.
 *
 * 128 * 64 / 8 = 1024 bytes.
 */
static uint8_t framebuffer[OLED_WIDTH * OLED_HEIGHT / 8];


/*
 * [OURS]
 *
 * Turn on one pixel in the SSD1306 framebuffer.
 *
 * The SSD1306 stores 8 vertical pixels inside one byte.
 */
static void oled_set_pixel(int x, int y)
{
    int page = y / 8;
    int bit  = y % 8;

    framebuffer[page * OLED_WIDTH + x] |= (1u << bit);
}


/*
 * [OURS]
 *
 * Convert a SIMP-style 256 x 256 virtual monitor into the physical
 * 128 x 64 SSD1306 framebuffer.
 *
 * This is intentionally simple for the learning stage:
 * - inspect each 4 x 4 block of SIMP pixels
 * - if any pixel is non-zero, turn on one OLED pixel
 * - 256 / 4 = 64, so the converted image is 64 x 64
 * - center it on the 128-pixel-wide OLED with a 32-pixel offset
 */
void oled_show_simp_monitor(const uint8_t *simp_monitor)
{
    memset(framebuffer, 0x00, sizeof(framebuffer));

    for (int oled_y = 0; oled_y < 64; oled_y++)
    {
        for (int oled_x = 0; oled_x < 64; oled_x++)
        {
            int pixel_on = 0;

            for (int dy = 0; dy < 4; dy++)
            {
                for (int dx = 0; dx < 4; dx++)
                {
                    int simp_x = oled_x * 4 + dx;
                    int simp_y = oled_y * 4 + dy;

                    int simp_index =
                        simp_y * SIMP_MONITOR_WIDTH
                        + simp_x;

                    if (simp_monitor[simp_index] != 0)
                    {
                        pixel_on = 1;
                    }
                }
            }

            if (pixel_on)
            {
                oled_set_pixel(
                    oled_x + 32,
                    oled_y
                );
            }
        }
    }

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

    ESP_ERROR_CHECK(esp_lcd_panel_reset(oled_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(oled_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(oled_panel, true, true));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(oled_panel, true));

    oled_clear();
}


void oled_fill(int on)
{
    if (on)
    {
        memset(framebuffer, 0xFF, sizeof(framebuffer));
    }
    else
    {
        memset(framebuffer, 0x00, sizeof(framebuffer));
    }

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
