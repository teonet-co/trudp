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
 * Snake network game.
 * Terminal module and basic function
 *
 * \file   snake.h
 * \author Kirill Scherba <kirill@scherba.ru>
 *
 * Created on June 24, 2016, 3:50 PM
 */

#include <stdio.h>
#include <stdint.h>
#include "libtrudp/utils_r.h"
#include "libtrudp/queue.h"
#include "libtrudp/packet.h"

//#define HL "-"
//#define VL "|"
//#define LTC "*"
//#define LBC "*"
//#define RTC "*"
//#define RBC "*"

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

static void show_line_horizontal(int x, int y, int width) {

    int i;
    gotoxy(x,y);
    for(i=0; i < width; i++)
        printf(HL);
}

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
    trudpQueue *snakes;
    
} scene;

static void show_scene(scene *sc, int width, int height, int *out_x, int *out_y) {
    
    if(!sc->initialized) {
        sc->snakes = trudpQueueNew();
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
    uint32_t last_key_pressed;
    int quited;

    void *s_matrix;

    trudpQueue *body; ///< Snake body queue

    struct termios oldt;

} snake;

static int check_snake(snake *sn, int x, int y) {
    
    int rv = 1;
    
    if(sn->x == x && sn->y == y) rv = 0;
    else {
        trudpQueueIterator *it = trudpQueueIteratorNew(sn->body);
        if(it != NULL) {
            while(trudpQueueIteratorNext(it)) {
                snake_body_data *b = (snake_body_data *)trudpQueueIteratorElement(it)->data;
                if(b->x == x && b->y == y) { 
                    rv = 0; 
                    break; 
                }
            }
            trudpQueueIteratorFree(it);
        }
    }    
    
    return rv;
}

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
        trudpQueueIterator *it = trudpQueueIteratorNew(sc->snakes);
        if(it != NULL) {
            while(trudpQueueIteratorNext(it)) {
                snake **sn_ptr = (snake **)trudpQueueIteratorElement(it)->data;
                snake *s = *sn_ptr;
                if(s != sn) {
                    if(!check_snake(s, x, y)) { 
                        rv = 0; 
                        break; 
                    }
                }
            }
            trudpQueueIteratorFree(it);
        }
    }
    
    return rv;
}

static void printf_snake(scene *sc, snake *sn) {

    // Print body
    trudpQueueIterator *it = trudpQueueIteratorNew(sn->body);
    if(it != NULL) {
        while(trudpQueueIteratorNext(it)) {
            snake_body_data *b = (snake_body_data *)trudpQueueIteratorElement(it)->data;
            gotoxy(sn->scene_left + b->x, sn->scene_top + b->y);
            printf("%s" SHAKE_BODY _ANSI_NONE,b->color);
        }
        trudpQueueIteratorFree(it);
    }
    // Print head
    gotoxy(sn->scene_left + sn->x, sn->scene_top + sn->y);
    printf("%s", sn->head_char);

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
       trudpGetTimestamp() - sn->last_key_pressed > 10000000 &&
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
        if(sn->auto_increment && sn->tic && !(sn->tic % sn->auto_increment)) {
            snake_body_data body;
            body.x = x;
            body.y = y;            
            body.color = _ANSI_NONE;
            trudpQueueAddTop(sn->body, (void*)&body, sizeof(snake_body_data));
            ((snake_body_data*)sn->body->last->data)->color = _ANSI_GREEN;
        }
        else if(trudpQueueSize(sn->body)) {
            ((snake_body_data *)sn->body->last->data)->x = x;
            ((snake_body_data *)sn->body->last->data)->y = y;
            ((snake_body_data *)sn->body->last->data)->color = _ANSI_NONE;
            trudpQueueMoveToTop(sn->body, sn->body->last);
        }
    }
    // Remove one body element from end
    else {
        trudpQueueDeleteLast(sn->body);
    }

}

void show_snake(scene *sc, snake *sn, int start_x, int start_y, int scene_left,
        int scene_top, int width, int height, int start_direction, 
        char* snake_head_char) {

    if(!sn->initialized || sn->quited) {
        // Hide cursor and Turn echoing off and fail if we can't.
        hidecursor();
        struct termios newt;
        tcgetattr(STDIN_FILENO, &sn->oldt);
        newt = sn->oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        
        sn->quited = 0;
    }
    
    if(!sn->initialized) {
        
        uint32_t ts = trudpGetTimestamp();
        srand(ts);

        sn->tic = 0;
        sn->head_char = snake_head_char;

        sn->x = start_x;
        sn->y = start_y;
        sn->auto_increment = 10;
        sn->auto_change_direction = 1;
        sn->random_direction = 1;
        sn->direction = start_direction;
        sn->last_key_pressed = ts;

        sn->s_width = width;
        sn->s_height = height;

        sn->initialized = 1;

        sn->body = trudpQueueNew();

        int i;
        const size_t START_BODY_LENGS = 9;
        for(i = 0; i < START_BODY_LENGS; i++) {
            snake_body_data body;
            body.x = start_x - i -1;
            body.y = start_y;
            body.color = _ANSI_NONE;
            trudpQueueAdd(sn->body, (void*)&body, sizeof(snake_body_data));
        }

        trudpQueueAdd(sc->snakes, (void*)&sn, sizeof(snake *));
    }
    else sn->tic++;

    sn->scene_left = scene_left;
    sn->scene_top = scene_top;

    // Printf snake (and calculate next position for bot snake)
    printf_snake(sc, sn);
}

static void restore_terminal(snake *sn) {

    // Show cursor and Turn echoing on
    showcursor();
    tcsetattr(STDIN_FILENO, TCSANOW, &sn->oldt);
    sn->quited = 1;
    cls();
}

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
        sn->last_key_pressed = trudpGetTimestamp(); 
    }

    return rv;
}

int run_snake() {

    int rv, x,y, width = 100, height = 50;
    static scene sc;
    static snake sn[4];

    // Show scene
    show_scene(&sc, width, height, &x, &y);

    // Initialize own snake and 3 bots
    show_snake(&sc, &sn[0], 10, 0, x, y, width, height, DI_RIGHT, SNAKE_HEAD);
    show_snake(&sc, &sn[1], 10, 10, x, y, width, height, DI_RIGHT, SNAKE_HEAD_OTHER);
    show_snake(&sc, &sn[2], 10, 20, x, y, width, height, DI_RIGHT, SNAKE_HEAD_OTHER);
    show_snake(&sc, &sn[3], 10, 30, x, y, width, height, DI_RIGHT, SNAKE_HEAD_OTHER);

    // Check key
    rv = check_key_snake(&sn);

    // Refresh screen
    fflush(stdout);

    // Restore terminal
    if(!rv) restore_terminal(&sn);

    return rv;
}
