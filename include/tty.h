
#ifndef TTY_H
#define TTY_H

#include <stdint.h>

void tty_init();

void tty_set_color(uint32_t color);
uint32_t tty_get_color(void);

void tty_print_char(char c);
void tty_print_string(char* s);


void puts_at_pos(char* s, uint32_t xpos, uint32_t ypos);
void puts_at_pos_transparent(char* s, uint32_t xpos, uint32_t ypos);

#endif