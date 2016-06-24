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
#include "libtrudp/utils_r.h"

#define LINE_HORIZONTAL_CHAR "-"
#define LINE_VERTICAL_CHAR "|"
#define LTC "*"
#define LBC "*"
#define RTC "*"
#define RBC "*"

#define SHAKE_HEAD "O"
#define SHAKE_BODY "X"

static void show_line_horizontal(int x, int y, int width) {
    
    int i; 
    gotoxy(x,y);
    for(i=0; i < width; i++) 
        printf(LINE_HORIZONTAL_CHAR);
}

static void show_line_vertical(int x, int y, int height) {
    
    int i; 
    for(i=0; i < height; i++, y++) { 
        gotoxy(x,y); 
        printf(LINE_VERTICAL_CHAR); 
    } 
}

// Show in terminal functions --------------------------------------------------
static void show_scene(int width, int height, int *out_x, int *out_y) {
       
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
    gotoxy(x,y-1); printf(LTC); // LTC
    gotoxy(x,y+height); printf(LBC); // LBC
    gotoxy(x+1+width,y-1); printf(RTC); // RTC
    gotoxy(x+1+width,y+height); printf(RBC); // RBC
    
    //fflush(stdout);
}

typedef enum snake_direction {
    
    DI_NO,
    DI_LEFT,
    DI_RIGHT,
    DI_UP,
    DI_DOWN
    
} snake_direction;

typedef struct snake {
    
    int initialized;
    
    int x;
    int y;
    
    int s_left;
    int s_top;
    int s_width;
    int s_height;
    
    int direction; ///< 0 - no; 1 - left; 2 - right; 3 - up; 4 - down
    
    void *s_matrix;
    void *body;
    
} snake;

static snake sn = { 0, 0, 0, 0, 0, 0, 0, 0, NULL };

void init_snake(snake *sn, int x, int y, int s_left, int s_top, int width, int height, int direction) {
    
    if(!sn->initialized) {
        
        sn->x = x;
        sn->y = y;        
        sn->direction = direction;
        
        sn->s_width = width;
        sn->s_height = height;
        
        sn->initialized = 1;
    }
    
    sn->s_left = s_left;
    sn->s_top = s_top;        
}

static int can_move_snake(snake *sn, int x, int y) {
    
    int rv = 1;
    
    // Check scena area
    if     (sn->s_left + x < sn->s_left) rv = 0;
    else if(sn->s_left + x >= sn->s_left + sn->s_width) rv = 0;
    else if(sn->s_top  + y < sn->s_top) rv = 0;
    else if(sn->s_top  + y >= sn->s_top + sn->s_height) rv = 0;
    
    return rv;
}

static void show_snake(snake *sn) {
    
    gotoxy(sn->s_left + sn->x, sn->s_top + sn->y); 
    
    switch(sn->direction) {
        case DI_LEFT:
        case DI_RIGHT:    
            if(sn->direction == DI_LEFT) printf(SHAKE_HEAD);
            printf(SHAKE_BODY);
            printf(SHAKE_BODY);
            printf(SHAKE_BODY);
            printf(SHAKE_BODY);
            printf(SHAKE_BODY);
            if(sn->direction == DI_RIGHT) printf(SHAKE_HEAD);
            break;
        case DI_UP:
        case DI_DOWN: {   
            int dif;
            printf(SHAKE_HEAD); 
            if(sn->direction == DI_UP) { dif = 1; } else dif = -1;
            gotoxy(sn->s_left + sn->x, sn->s_top + sn->y + dif); printf(SHAKE_BODY); 
            gotoxy(sn->s_left + sn->x, sn->s_top + sn->y + dif*2); printf(SHAKE_BODY); 
            gotoxy(sn->s_left + sn->x, sn->s_top + sn->y + dif*3); printf(SHAKE_BODY); 
            gotoxy(sn->s_left + sn->x, sn->s_top + sn->y + dif*4); printf(SHAKE_BODY); 
            gotoxy(sn->s_left + sn->x, sn->s_top + sn->y + dif*5); printf(SHAKE_BODY); 
            //if(sn->direction == DI_DOWN) { gotoxy(sn->s_left + sn->x, sn->s_top + sn->y); printf(SHAKE_HEAD); }
        } break;
    }
    //fflush(stdout);
    
    // Calculate next head position
    switch(sn->direction) {        
        case DI_LEFT:  if(can_move_snake(sn, sn->x-1, sn->y)) sn->x--; else sn->direction = DI_UP;    break;
        case DI_RIGHT: if(can_move_snake(sn, sn->x+1, sn->y)) sn->x++; else sn->direction = DI_DOWN;  break;
        case DI_UP:    if(can_move_snake(sn, sn->x, sn->y-1)) sn->y--; else sn->direction = DI_RIGHT; break;
        case DI_DOWN:  if(can_move_snake(sn, sn->x, sn->y+1)) sn->y++; else sn->direction = DI_LEFT;  break;
        default: break;
    }
}

void run_snake() {
    
    int x,y, width = 100, height = 50;
    
    hidecursor();

    // Show scene
    show_scene(width, height, &x, &y);
    
    // Initialize snake 
    init_snake(&sn, 7, 0, x, y, width, height, DI_RIGHT);

    // Show snake 
    show_snake(&sn);  
    
    // Refresh screen
    fflush(stdout);
}
