#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include "game.h"

void draw_board(ALLEGRO_DISPLAY *display, ALLEGRO_BITMAP *bitmap);
void draw_x(ALLEGRO_DISPLAY *display, ALLEGRO_BITMAP *bitmap,unsigned char row, unsigned char col);
void draw_o(ALLEGRO_DISPLAY *display, ALLEGRO_BITMAP *bitmap, unsigned char row, unsigned char col);

ALLEGRO_MOUSE_EVENT get_mouse_click_event(ALLEGRO_EVENT_QUEUE *event_queue);
unsigned char check_click_out_of_bounds(ALLEGRO_DISPLAY *display, ALLEGRO_MOUSE_EVENT click_event);
Cell get_cell_from_click_event(ALLEGRO_DISPLAY *display, ALLEGRO_MOUSE_EVENT click_event);
void display_winner_banner(ALLEGRO_DISPLAY *display);

#endif
