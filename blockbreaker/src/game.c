#include "game.h"
#include "utils.h"

#define NUM_OF_BLOCKS(x) (3 - (x))
#define MAX_BALLS 5

pthread_mutex_t ncurses_mutex = PTHREAD_MUTEX_INITIALIZER;
unsigned char game_area[WINDOW_LENGTH][WINDOW_HEIGHT];
Ball *balls[MAX_BALLS];
int num_balls = 0;

void initialize_game() {
	// Seed the random number generator
	srand(time(NULL));
	WINDOW *win = initialize_screen();
	game_loop(win);
}

Ball *generate_ball(Coordinate *coordinate) {
	if (num_balls > MAX_BALLS) {
		return NULL;
	}

	Ball *ball = malloc(sizeof(Ball));
	if (ball == NULL) {
		// Handle error: malloc failed
		return NULL;
	}
	ball->body.x = coordinate->x;
	ball->body.y = coordinate->y;
	ball->direction = 3;

	// Add the new ball to the global list of balls
	balls[num_balls] = ball;
	num_balls++;

	return ball;
}

void display_ball(WINDOW *window, Ball *ball) {
	mvwaddch(window, ball->body.y, ball->body.x, 'O');
	wrefresh(window);
}

void move_ball(WINDOW *window, Ball *ball) {
	switch (ball->direction) {
		// up direction
		case 0:
			mvwaddch(window, ball->body.y, ball->body.x, ' ');	
			ball->body.y--;
			break;
		// down direction
		case 1:
			mvwaddch(window, ball->body.y, ball->body.x, ' ');	
			ball->body.y++;
			break;
		// left diagonal up
		case 2:
			mvwaddch(window, ball->body.y, ball->body.x, ' ');	
			ball->body.y--;
			ball->body.x--;
			break;
		// right diagonal up
		case 3:
			mvwaddch(window, ball->body.y, ball->body.x, ' ');	
			ball->body.y--;
			ball->body.x++;
			break;
		// left diagonal down
		case 4:
			mvwaddch(window, ball->body.y, ball->body.x, ' ');	
			ball->body.y++;
			ball->body.x--;
			break;
		// right diagonal down
		case 5:
			mvwaddch(window, ball->body.y, ball->body.x, ' ');	
			ball->body.y++;
			ball->body.x++;
			break;
		default:
			break;
	}
	display_ball(window, ball);
}

void construct_level(WINDOW *window, unsigned char level) {
	// how many blocks to display
	Coordinate coordinate;
	coordinate.x = getmaxx(window)/2 - 2;
	coordinate.y = getmaxy(window) - 5;
	Ball *ball = generate_ball(&coordinate);
	//display_ball(window, ball);
	for(unsigned char i = 0; i < NUM_OF_BLOCKS(level); i++ ) {
		Block *block = generate_block(window);
		display_block(window, block);
	}
}

Block* generate_block(WINDOW *window) {
	Block* block = malloc(sizeof(Block));
	block->length = 3;
	block->body = malloc(block->length * sizeof(Coordinate));
	block->health = 1;
	Coordinate coordinate;
	do {
		coordinate = get_random_coordinates(window);
	} while (is_overlap_block(coordinate, block->length));
	for (unsigned char i = 0; i < block->length; i++) {
		block->body[i].x = coordinate.x + i;
		block->body[i].y = coordinate.y;
		game_area[block->body[i].x][block->body[i].y] = 1;  // Mark this coordinate as occupied
	}
	return block;
}

void check_for_collision_with_block(Ball *ball) {
	if (is_overlap(ball->body) == 1) {
		switch (ball->direction) {
			case 0:
				ball->direction = 1;
				break;
			case 1:
				ball->direction = 0;
				break;
			case 2:
				ball->direction = 4;
				break;
			case 3:
				ball->direction = 5;
				break;
			case 4:
				ball->direction = 2;
				break;
			case 5:
				ball->direction = 3;
				break;
		}
		Coordinate new_coordinate;
		new_coordinate.x = ball->body.x;
		new_coordinate.y = ball->body.y;
		generate_ball(&new_coordinate);
		game_area[ball->body.x][ball->body.y] = 0;
	}
}

// this one is used for the ball hitting the blocks
unsigned char is_overlap(Coordinate coordinate) {
	if (game_area[coordinate.x][coordinate.y] == 1) {
		return 1;  // There is an overlap
	}
	return 0; // No overlap
}

// this is only for the purpose of blocks not being generated on top of each other
unsigned char is_overlap_block(Coordinate coordinate, unsigned char length) {
	for (unsigned char i = 0; i < length; i++) {
		if (game_area[coordinate.x + i][coordinate.y == 1]) {
			return 1;  // There is an overlap
		}
	}
	return 0;  // No overlap
}

void check_for_collision_with_bar(Ball *ball, Bar *bottom_bar) {
	if ((ball->body.x == bottom_bar->body[0].x + bottom_bar->length / 2) && (ball->body.y == bottom_bar->body[0].y - 1)) {
		ball->direction = 0;
	} else if ((ball->body.x >= bottom_bar->body[0].x - 1 && ball->body.x < bottom_bar->body[0].x + bottom_bar->length / 2) && ball->body.y == bottom_bar->body[0].y - 1)
	{
		ball->direction = 2;
	} else if ((ball->body.x > bottom_bar->body[0].x + bottom_bar->length / 2) && (ball->body.x <= bottom_bar->body[0].x + bottom_bar->length) && (ball->body.y == bottom_bar->body[0].y - 1)) {
		ball->direction = 3;
	}
}

unsigned char check_for_collision_boundaries(WINDOW *window,Ball *ball) {
	// if the ball is hitting the top boundary
	if (ball->body.y <= 1) {
		if (ball->direction == 0) {
			ball->direction = 1;
		}
		else if (ball->direction == 2) {
			ball->direction = 4;
		}
		else if (ball->direction == 3) {
			ball->direction = 5;
		}
	}
	// if the ball is hitting the bottom boundary game over
	else if (ball->body.y >= getmaxy(window)) {
		return 1;
	}
	else if (ball->body.x <= 1) {
		if (ball->direction == 0 || ball->direction == 2) {
			ball->direction = 3;
		} 
		else if (ball->direction == 1 || ball->direction == 4) {
			ball->direction = 5;
		}
	}
	// if ball is hitting the right side
	else if (ball->body.x >= getmaxx(window) - 2) {
		if (ball->direction == 0 || ball->direction == 3) {
			ball->direction = 2;
		} else if (ball->direction == 1 || ball->direction == 5) {
			ball->direction = 4;
		}
	}
	return 0;
}

void display_block(WINDOW *window, Block *block) {
	// Display the block
	for (unsigned char i = 0; i < block->length; i++) {
		mvwaddch(window, block->body[i].y, block->body[i].x, ACS_CKBOARD);
	}
	wrefresh(window);
}

Bar *generate_bottom_bar(WINDOW *window) {
	Bar *bar = malloc(sizeof(Bar));
	bar->length = 5;
	bar->body = malloc(bar->length * sizeof(Coordinate));
	
	for (unsigned char i = 0; i < bar->length; i++) {
		bar->body[i].x = getmaxx(window)/2 - 2 + i;
		bar->body[i].y = getmaxy(window) - 2;
	}
	return bar;
}

void display_bottom_bar(WINDOW *window, Bar* bottom_bar) {
	// Display the block
	for (unsigned char i = 0; i < bottom_bar->length; i++) {
		mvwaddch(window, bottom_bar->body[i].y, bottom_bar->body[i].x, ACS_CKBOARD);
	}
	wrefresh(window);
}

void handle_user_input(WINDOW *window, Bar *bottom_bar) {
	int ch = getch();
	// If there's no input, ch will be ERR
	if (ch != ERR) {
		// Handle user input
		switch (ch) {
			case KEY_RIGHT:
				// check if the bottom bar is moving out of the window’s boundaries
				if (bottom_bar->body[bottom_bar->length - 1].x >= getmaxx(window) - 2) {
					break;
				}
				// erase one block to make it seem moving
				mvwaddch(window, bottom_bar->body[0].y, bottom_bar->body[0].x, ' ');
				// move the entire bar right
				for (unsigned char i = 0; i < bottom_bar->length; i++) {
					bottom_bar->body[i].x++;
				}
				break;
			case KEY_LEFT:
				// check if the bottom bar is moving out of the window’s boundaries
				if (bottom_bar->body[0].x <= 1) {
					break;
				}
				// erase one block to make it seem moving
				mvwaddch(window, bottom_bar->body[bottom_bar->length - 1].y, bottom_bar->body[bottom_bar->length - 1].x, ' ');
				// move the entire bar left
				for (unsigned char i = 0; i < bottom_bar->length; i++) {
					bottom_bar->body[i].x--;
				}
				break;
			default:
				break;
		}
	}
	pthread_mutex_lock(&ncurses_mutex);
	display_bottom_bar(window, bottom_bar);
	pthread_mutex_unlock(&ncurses_mutex);
}

void *handle_user_input_thread(void *args) {
	// Unpack the arguments
	struct handle_user_input_thread_args *targs = (struct handle_user_input_thread_args *)args;
	WINDOW *window = targs->window;
	Bar *bottom_bar = targs->bottom_bar;

	while (1) {
		handle_user_input(window, bottom_bar);
	}
	return NULL;
}

/* if game area is all zeroes then all the blocks have been destroyed and level is over
returns 1 if there is still blocks left
returns 0 when all the blocks are gone
*/
unsigned char game_area_clean() {
	for (unsigned char i = 0; i < WINDOW_LENGTH; i++) {
		for (unsigned char j = 0; j < WINDOW_HEIGHT; j++) {
			if (game_area[i][j] == 1) {
				return 1;
			}
		}
	}
	return 0;
}

unsigned char monitor_level(WINDOW *window, Bar *bottom_bar) {
	// Create a struct to hold the arguments for the new thread
	struct handle_user_input_thread_args targs;
	targs.window = window;
	targs.bottom_bar = bottom_bar;

	// Create the new thread
	pthread_t input_thread;
	pthread_create(&input_thread, NULL, handle_user_input_thread, &targs);

	while (game_area_clean() == 1) {
		pthread_mutex_lock(&ncurses_mutex);
		for (int i = 0; i < num_balls; i++) {
			move_ball(window, balls[i]);
			// debug(window, ball);
		}
		pthread_mutex_unlock(&ncurses_mutex);
		for (int i = 0; i < num_balls; i++) {
			check_for_collision_with_block(balls[i]);
			check_for_collision_with_bar(balls[i], bottom_bar);

			if (check_for_collision_boundaries(window, balls[i]) == 1) {
				free(balls[i]);
				// Shift all the balls after this one to the left in the array
				for (int j = i; j < num_balls - 1; j++) {
					balls[j] = balls[j + 1];
				}
				num_balls--;
				// Decrease i so that the next iteration checks the same index again
				i--;
			}
		}
		pthread_mutex_lock(&ncurses_mutex);
		wrefresh(window);
		pthread_mutex_unlock(&ncurses_mutex);
		// no more balls left game over
		if (num_balls <= 0) {
			return 1;
		}
		usleep(100000);
	}
	return 0;
}

void game_loop(WINDOW* window) {
	unsigned char current_level = 1;
	unsigned char level_over = 0;
	Bar *bottom_bar = generate_bottom_bar(window);
	display_bottom_bar(window, bottom_bar);

	// if monitor level returns 1 then the ball went below the bar, game over
	// if it returns 0 then the user has broken all the blocks and proceeds to next level
	while (1) {
		construct_level(window, current_level);
		if (monitor_level(window, bottom_bar) == 1) {
			print_the_end(window, current_level);
		} else if (monitor_level(window, bottom_bar) == 0) {
			current_level++;
		}
	}
	// Clean up and close
	endwin();
}

void print_the_end(WINDOW *window, unsigned char current_level) {
	// move the cursor to the center of the screen
	move(getmaxy(window) / 2, getmaxx(window) / 2 - 10);
	printw("Game Over! Your Score: %d", current_level);

	// Below line is used for the message to be printed to stdout otherwise sleep comes first
	refresh();

	// Refresh the screen to update the changes
	wrefresh(window);
	// Wait for a key press before exiting
	usleep(1500000);
	nodelay(stdscr, FALSE);
	getch();
}

void debug(WINDOW* window, Ball *ball) {
	// Move the cursor to the bottom of the window and print debugging information
	move(getmaxy(window), 0);
	clrtoeol();  // Clear the current line
	printw("Ball is currently at: %d, %d\n", ball->body.x, ball->body.y);
	printw("Maxx: %d Maxy: %d", getmaxx(window), getmaxy(window));
	wrefresh(window);
}
