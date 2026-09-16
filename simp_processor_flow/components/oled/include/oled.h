#pragma once

#include <stdint.h>

void oled_init(void);
void oled_fill(int on);
void oled_clear(void);

/*
 * Display a 256 x 256 SIMP virtual monitor on the 128 x 64 OLED.
 *
 * Current learning-stage mapping:
 * - each 4 x 4 SIMP block becomes one OLED pixel
 * - resulting 64 x 64 image is centered horizontally
 * - any non-zero SIMP pixel becomes white
 */
void oled_show_simp_monitor(const uint8_t *simp_monitor);
