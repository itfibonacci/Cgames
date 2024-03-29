#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include "game.h"

unsigned char check_winning_condition(Game *game);
unsigned char check_winning_condition_row(Game *game);
unsigned char check_winning_condition_column(Game *game);
unsigned char check_winning_condition_diagonal_left_right(Game *game);
unsigned char check_winning_condition_diagonal_right_left(Game *game);
unsigned char check_for_draw(Game *game);

#endif
