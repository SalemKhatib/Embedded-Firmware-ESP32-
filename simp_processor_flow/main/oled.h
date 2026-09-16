#pragma once

#include <stdint.h>

void oled_init(void);
void oled_fill(int on);
void oled_clear(void);

void oled_show_simp_monitor(const uint8_t *simp_monitor);