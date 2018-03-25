/*
 * The MIT License
 *
 * Copyright 2016 Kirill Scherba <kirill@scherba.ru>.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/*
 * Console Snake game.
 *
 * \file   snake.h
 * \author Kirill Scherba <kirill@scherba.ru>
 *
 * Created on June 24, 2016, 3:50 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "utils_r.h"
#include "queue.h"
#include "packet.h"

// Box characters
#define RB "\e(0\x6a\e(B" // 188 Right Bottom corner
#define RT "\e(0\x6b\e(B" // 187 Right Top corner
#define LT "\e(0\x6c\e(B" // 201 Left Top cornet
#define LB "\e(0\x6d\e(B" // 200 Left Bottom corner
#define MC "\e(0\x6e\e(B" // 206 Midle Cross
#define HL "\e(0\x71\e(B" // 205 Horizontal Line
#define LC "\e(0\x74\e(B" // 204 Left Cross
#define RC "\e(0\x75\e(B" // 185 Right Cross
#define BC "\e(0\x76\e(B" // 202 Bottom Cross
#define TC "\e(0\x77\e(B" // 203 Top Cross
#define VL "\e(0\x78\e(B" // 186 Vertical Line
#define SP " " 		  // space string

#define SNAKE_HEAD "O"
#define SNAKE_HEAD_OTHER "H"
#define SHAKE_BODY "X"

/**
 * Print horizontal line
 * 
 * @param x X position
 * @param y Y position
 * @param width Line width
 */
static void show_line_horizontal(int x, int y, int width) {

    int i;
    gotoxy(x,y);
    for(i=0; i < width; i++)
        printf(HL);
}

/**
 * Show vertical line
 * 
 * @param x X position
 * @param y Y position
 * @param height Line height
 */
static void show_line_vertical(int x, int y, int height) {

    int i;
    for(i=0; i < height; i++, y++) {
        gotoxy(x,y);
        printf(VL);
    }
}

// Show in terminal functions --------------------------------------------------
typedef struct scene {
    
    int initialized;

    int scene_left;
    int scene_top;
    char *matrix;    
    int width;
    int height;
    teoQueue *snakes;
    
} scene;

/**
 * Get matrix character at x,y position 
 * 
 * @param sc Pointer to scena
 * @param x X position
 * @param y Y position
 * @return 
 */
static char get_matrix_char(scene *sc, int x, int y) {
    
    return *(sc->matrix + (x) + (y) * sc->width);
}

/**
 * Set matrix character at x,y position 
 * 
 * @param sc Pointer to scena
 * @param x X position
 * @param y Y position
 * @param ch Character
 * @return 
 */
static void set_matrix_char(scene *sc, int x, int y, int ch) {
    
    *(sc->matrix + (x) + (y) * sc->width) = (char)ch;
}

/**
 * Print matrix char
 * 
 * @param sc Pointer to scena
 * @param x X position
 * @param y Y position
 */
static void show_matrix_char(scene *sc, int x, int y) {
    int ch = get_matrix_char(sc, x, y);
    if(ch != ' ') {
        gotoxy(sc->scene_left + x, sc->scene_top + y);
        printf("%c", ch);
    }
}

/**
 * Print matrix
 * 
 * @param sc Pointer to scena
 */
static void show_matrix(scene *sc) {
    int i, j;
    for(i = 0; i < sc->width; i++)
        for(j = 0; j < sc->height; j++)
            show_matrix_char(sc, i, j);
}

/**
 * Generate and print food
 * 
 * @param sc
 */
static void generate_food(scene *sc) {
    
    static int tic = 0;
    if(tic++ && !(tic % 20)) {
            
        int x = rand() % sc->width;
        int y = rand() % sc->height;
        if(get_matrix_char(sc, x, y) == ' ') {
            set_matrix_char(sc, x, y, '*');
            show_matrix_char(sc, x, y);
        }
    }
}

/**
 * Initialize and(or) print scene
 * 
 * @param sc
 * @param width
 * @param height
 * @param matrix
 * @param out_x
 * @param out_y
 */
static void show_scene(scene *sc, int width, int height, char*matrix, 
        int *out_x, int *out_y) {
    
    if(!sc->initialized) {
        sc->snakes = teoQueueNew();
        sc->width = width;
        sc->height = height;
        sc->matrix = matrix;
        
        { // Initialize the matrix 
            int i, j;
            for(i = 0; i < sc->width; i++)
                for(j = 0; j < sc->height; j++)
                    set_matrix_char(sc, i, j, ' ');
        }
        
        sc->initialized = 1;
    }

    int cols = tcols(), rows = trows();
    int x = (cols-width) / 2, y = (rows-height-1) / 2;

    // Title
    cls();
    gotoxy(x,y);
    printf("SNAKE GAME"); y++;

    // Return scene position
    if(out_x) *out_x = x+1;
    if(out_y) *out_y = y+1;
    sc->scene_left = x+1;
    sc->scene_top = y+1;

    // Top line
    show_line_horizontal(x+1,y,width); y++;

    // Left line
    show_line_vertical(x,y,height);

    // Right line
    show_line_vertical(x+1+width,y,height);

    // Bottom line
    show_line_horizontal(x+1,y+height,width);

    // Show corners
    gotoxy(x,y-1); printf(LT); // LTC
    gotoxy(x,y+height); printf(LB); // LBC
    gotoxy(x+1+width,y-1); printf(RT); // RTC
    gotoxy(x+1+width,y+height); printf(RB); // RBC
    
    // Show matrix
    show_matrix(sc);
    
    // Generate food    
    generate_food(sc);
}

typedef enum snake_direction {

    DI_NO,
    DI_LEFT,
    DI_RIGHT,
    DI_UP,
    DI_DOWN

} snake_direction;

typedef struct snake_body_data {

    int x;
    int y;
    char *color;

} snake_body_data;

typedef struct snake {

    int initialized;

    char *head_char;
    int x;
    int y;

    int scene_left;
    int scene_top;
    int s_width;
    int s_height;

    int auto_change_direction;
    int random_direction;
    int direction; ///< 0 - no; 1 - left; 2 - right; 3 - up; 4 - down
    int auto_increment;
    uint32_t tic;
    uint64_t last_key_pressed;
    int quited;

    void *s_matrix;

    teoQueue *body; ///< Snake body queue

    #if !defined(_WIN32) && !defined(_WIN64)
    struct termios oldt;
    #endif

} snake;

/**
 * Check that snake is at position
 * 
 * @param sn Pointer to snake
 * @param x X position
 * @param y Y position
 * 
 * @return Return 0 if snake is at x,y position
 */
static int check_snake(snake *sn, int x, int y) {
    
    int rv = 1;
    
    if(sn->x == x && sn->y == y) rv = 0;
    else {
        teoQueueIterator *it = teoQueueIteratorNew(sn->body);
        if(it != NULL) {
            while(teoQueueIteratorNext(it)) {
                snake_body_data *
                b = (snake_body_data *)teoQueueIteratorElement(it)->data;
                if(b->x == x && b->y == y) { 
                    rv = 0; 
                    break; 
                }
            }
            teoQueueIteratorFree(it);
        }
    }    
    
    return rv;
}

/**
 * Can move snake. Check scene and snakes (include himself)
 * 
 * @param sc Pointer to scene
 * @param sn Pointer to snake
 * @param x X position
 * @param y Y position
 * 
 * @return True if can move to x,y position
 */
static int can_move_snake(scene *sc, snake *sn, int x, int y) {

    int rv = 1;

    // Check scena area
    if     (sn->scene_left + x < sn->scene_left) rv = 0;
    else if(sn->scene_left + x >= sn->scene_left + sn->s_width) rv = 0;
    else if(sn->scene_top  + y < sn->scene_top) rv = 0;
    else if(sn->scene_top  + y >= sn->scene_top + sn->s_height) rv = 0;
    
    // Check himself
    if(rv) {
        rv = check_snake(sn, x, y);
    }
    
    // Check other snakes
    if(rv) {
        teoQueueIterator *it = teoQueueIteratorNew(sc->snakes);
        if(it != NULL) {
            while(teoQueueIteratorNext(it)) {
                snake **sn_ptr = (snake **)teoQueueIteratorElement(it)->data;
                snake *s = *sn_ptr;
                if(s != sn) {
                    if(!check_snake(s, x, y)) { 
                        rv = 0; 
                        break; 
                    }
                }
            }
            teoQueueIteratorFree(it);
        }
    }
    
    return rv;
}

/**
 * Can eat. Check matrix position and remove food if it present
 * 
 * @param sc Pointer to scene
 * @param sn Pointer to snake
 * @param x X position
 * @param y Y position
 * @return 
 */
static int can_eat(scene *sc, snake *sn, int x, int y) {
    
    int rv = 0;
    
    if(get_matrix_char(sc, x, y) == '*') {
        set_matrix_char(sc, x, y, ' ');
        
        rv = 1;
    }
    
    return rv;
}

/**
 * Increment snake
 * 
 * @param sn Pointer to snake
 * @param x X position
 * @param y Y position
 */
static void increment_snake(snake *sn, int x, int y) {
    
    snake_body_data body;
    body.x = x;
    body.y = y;            
    body.color = _ANSI_NONE;
    teoQueueAddTop(sn->body, (void*)&body, sizeof(snake_body_data));
    ((snake_body_data*)sn->body->last->data)->color = _ANSI_GREEN;
}

/**
 * Print snake and calculate next position
 * 
 * @param sc Pointer to scene
 * @param sn Pointer to snake
 */
static void printf_snake(scene *sc, snake *sn) {

    // Print body
    teoQueueIterator *it = teoQueueIteratorNew(sn->body);
    if(it != NULL) {
        while(teoQueueIteratorNext(it)) {
            snake_body_data *b = (snake_body_data *)teoQueueIteratorElement(it)->data;
            gotoxy(sn->scene_left + b->x, sn->scene_top + b->y);
            printf("%s" SHAKE_BODY _ANSI_NONE,b->color);
        }
        teoQueueIteratorFree(it);
    }
    // Print head
    gotoxy(sn->scene_left + sn->x, sn->scene_top + sn->y);
    printf("%s", sn->head_char);
    
    // Eat matrix element
    int do_can_eat = can_eat(sc, sn, sn->x, sn->y);

    // Calculate next head position
    int can_move = 0, x = sn->x, y = sn->y, direction = sn->direction;
    switch(sn->direction) {
        case DI_LEFT:
            if(can_move_snake(sc, sn, sn->x-1, sn->y)) { can_move = 1; sn->x--; }
            else if(sn->auto_change_direction) sn->direction = DI_UP;
            break;
        case DI_RIGHT:
            if(can_move_snake(sc, sn, sn->x+1, sn->y)) { can_move = 1; sn->x++; }
            else if(sn->auto_change_direction) sn->direction = DI_DOWN;
            break;
        case DI_UP:
            if(can_move_snake(sc, sn, sn->x, sn->y-1)) { can_move = 1; sn->y--; }
            else if(sn->auto_change_direction) sn->direction = DI_RIGHT;
            break;
        case DI_DOWN:
            if(can_move_snake(sc, sn, sn->x, sn->y+1)) { can_move = 1; sn->y++; }
            else if(sn->auto_change_direction) sn->direction = DI_LEFT;
            break;
        default: break;
    }
    
    // Random direction
    if(sn->random_direction && 
       sn->auto_change_direction && 
       direction == sn->direction &&
       teoGetTimestampFull() - sn->last_key_pressed > 10000000 &&
       sn->tic && !(sn->tic % 10) 
            ) {
        
        int new_direction = rand() % 4 + 1;
        if( !((sn->direction == DI_LEFT && new_direction == DI_RIGHT) ||
              (sn->direction == DI_RIGHT && new_direction == DI_LEFT) ||     
              (sn->direction == DI_UP && new_direction == DI_DOWN) ||     
              (sn->direction == DI_DOWN && new_direction == DI_UP)) ) {
            
            sn->direction = new_direction;
        }     
    }

    // Move body end to first position    
    if(can_move) {
        // Auto increment
        if(do_can_eat || (sn->auto_increment && sn->tic && !(sn->tic % sn->auto_increment))) {
            increment_snake(sn, x, y);
        }
        else if(teoQueueSize(sn->body)) {
            ((snake_body_data *)sn->body->last->data)->x = x;
            ((snake_body_data *)sn->body->last->data)->y = y;
            ((snake_body_data *)sn->body->last->data)->color = _ANSI_NONE;
            teoQueueMoveToTop(sn->body, sn->body->last);
        }
    }
    // Remove one body element from end
    else {
        teoQueueDeleteLast(sn->body);
    }
}

/**
 * Echo off (and save terminal configuration)
 * 
 * @param sn Pointer to snake
 */
static void echo_off(snake *sn) {
    
    #if !defined(_WIN32) && !defined(_WIN64)
    struct termios newt;
    tcgetattr(STDIN_FILENO, &sn->oldt);
    newt = sn->oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    #endif
}

/**
 * Restore terminal configuration echo
 * @param sn
 */
static void restore_terminal(snake *sn) {

    // Show cursor and Turn echoing on
    showcursor();
    #if !defined(_WIN32) && !defined(_WIN64)
    tcsetattr(STDIN_FILENO, TCSANOW, &sn->oldt);
    sn->quited = 1;
    cls();
    #endif
}

/**
 * Initialize and(or) print snake
 * 
 * @param sc
 * @param sn
 * @param start_x
 * @param start_y
 * @param scene_left
 * @param scene_top
 * @param width
 * @param height
 * @param start_direction
 * @param snake_head_char
 */
void show_snake(scene *sc, snake *sn, int start_x, int start_y, int scene_left,
        int scene_top, int width, int height, int start_direction, 
        char* snake_head_char) {

    if(!sn->initialized || sn->quited) {
        // Hide cursor and Turn echoing off and fail if we can't.
        hidecursor();
        echo_off(sn);
        
        sn->quited = 0;
    }
    
    if(!sn->initialized) {
        
        uint64_t ts = teoGetTimestampFull();

        sn->tic = 0;
        sn->head_char = snake_head_char;

        sn->x = start_x;
        sn->y = start_y;
        sn->auto_increment = *snake_head_char == SNAKE_HEAD[0] ? 0 : 20;
        sn->auto_change_direction = 1;
        sn->random_direction = 1;
        sn->direction = start_direction;
        sn->last_key_pressed = ts;

        sn->s_width = width;
        sn->s_height = height;

        sn->initialized = 1;

        sn->body = teoQueueNew();

        int i;
        const size_t START_BODY_LENGS = 9;
        for(i = 0; i < START_BODY_LENGS; i++) {
            snake_body_data body;
            body.x = start_x - i -1;
            body.y = start_y;
            body.color = _ANSI_NONE;
            teoQueueAdd(sn->body, (void*)&body, sizeof(snake_body_data));
        }

        teoQueueAdd(sc->snakes, (void*)&sn, sizeof(snake *));
    }
    else sn->tic++;

    sn->scene_left = scene_left;
    sn->scene_top = scene_top;

    // Printf snake (and calculate next position for bot snake)
    printf_snake(sc, sn);
}

/**
 * Check key pressed (to move snake)
 * 
 * @param sn Pointer to snake
 * @return Pressed key code
 * @retval up - 14, down - 15, left - 16, right - 17 key code
 * @retval q,s - quit snake
 * @retval 0 - any other key
 */
static int check_key_snake(snake *sn) {

    int rv = 1;

    if(kbhit()) {
        int ch = getkey();
        switch(ch) {
            case 14 /*KEY_UP*/:   sn->direction = DI_UP;       break;
            case 15 /*KEY_DOWN*/: sn->direction = DI_DOWN;     break;
            case 16 /*KEY_LEFT*/: sn->direction = DI_LEFT;     break;
            case 17 /*KEY_RIGHT*/: sn->direction = DI_RIGHT;   break;
            case 'q': case 's': rv = 0; break;
            default: break;
        }
        sn->last_key_pressed = teoGetTimestampFull(); 
    }

    return rv;
}

/**
 * Run snake (one tick)
 * 
 * @return 
 */
int run_snake() {

    srand(teoGetTimestampFull());

    static scene sc;
    static snake sn[4];
    int rv, x,y; 
    #define WIDTH 100
    #define HEIGHT 50    
    static char matrix[(WIDTH+1)*(HEIGHT+1)];
    
    // Show scene
    show_scene(&sc, WIDTH, HEIGHT, (char*)matrix, &x, &y);

    // Initialize own snake and 3 bots
    show_snake(&sc, &sn[0], 10, 0, x, y, WIDTH, HEIGHT, DI_RIGHT, SNAKE_HEAD);
    show_snake(&sc, &sn[1], 10, 10, x, y, WIDTH, HEIGHT, DI_RIGHT, SNAKE_HEAD_OTHER);
    show_snake(&sc, &sn[2], 10, 20, x, y, WIDTH, HEIGHT, DI_RIGHT, SNAKE_HEAD_OTHER);
    show_snake(&sc, &sn[3], 10, 30, x, y, WIDTH, HEIGHT, DI_RIGHT, SNAKE_HEAD_OTHER);

    // Check key
    rv = check_key_snake(&sn[0]);

    // Refresh screen
    fflush(stdout);

    // Restore terminal
    if(!rv) restore_terminal(&sn[0]);

    return rv;
}
