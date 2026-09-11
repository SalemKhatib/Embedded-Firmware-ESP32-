#pragma once

void button_init(void);
int button_is_pressed(void);
int button_event_pending(void);
void button_clear_event(void);
void button_wait_for_release(void);
