/*
 * Copyright (C) 2024 Thomas Mayr <mayr.t@aon.at>
 * Copyright (C) 2024 Johannes Artelsmair <johannes.artelsmair@gmx.at>
 *
 * SPDX-License-Identifier: MIT
 */


#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "graphics.h"
#include "bitmaps.h"
#include "irq.h"

#define FRAME_WIDTH 800
#define FRAME_HEIGHT 480
#define FRAME_BUF_SIZE (FRAME_WIDTH * FRAME_HEIGHT * 2)

#define KEY_PRESSED *VNC_KBD_KEY_ADDR & 1
#define KEY_DATA_AVAIL *VNC_KBD_KEY_ADDR & (1 << 31)

static uint16_t * const VNC_FRAME_BUF_ADDR = (uint16_t * const)0x11000000;
static volatile uint32_t * const VNC_KBD_CTRL_ADDR = (uint32_t * const)0x12001000;
static volatile uint32_t * const VNC_KBD_KEY_ADDR = (uint32_t * const)0x12001004;

static uint16_t BACKGROUND [FRAME_WIDTH*FRAME_HEIGHT] = {0};

enum Direction {
  RIGHT,
  LEFT
};


struct position{
    int x_pos;
    int y_pos;
};

struct invader{
    bool alive; //if the invader is alive
    Bitmap *bitmap[2]; //pointers to the bitmaps
};

uint16_t framebuf[FRAME_WIDTH*FRAME_HEIGHT] = {0};
int dead_rows = 0;

struct invader invaders_s[11][5];
struct position projectile_positions[5];
struct position invaders_position;

/*
The function checks how many empty rows of shot invaders are there already
*/
void check_dead_rows() // the function checks how many empty rows of shot invaders are there already
{       int i, j = 0;
        for ( i = 0; i<5; i++)
        {
            j = 0;
            for ( j = 0; j<11; j++) // now check every entry of the row
            {
                if(invaders_s[j][4-i].alive == true)
                    break;
            }
            if(j < 10)
            break;
        }
    dead_rows = i;
}
/*
printing all invaders, which are alive onto the screenbuffer
*/
void print_invaders(int x_pos, int y_pos, int alien_type)
{
    uint16_t invader_buf[FRAME_WIDTH*(y_pos+264)]; //create an extra framebuffer for the invaders, so  they ccan  be copied at once.
    memset( invader_buf, 0, FRAME_WIDTH*(y_pos+264)*sizeof(uint16_t) );
    int i = 0, j, y_p;
    for(j = 0; j< 5; j++)
    {
        for(i = 0; i<11; i++)
        {
            if(invaders_s[i][j].alive == true) //check if invader is already dead
            copy_bitmap ((uint16_t*)&invader_buf, invaders_s[i][j].bitmap[alien_type], x_pos + i*48, y_pos+48*j);
        }
    }
    memcpy(VNC_FRAME_BUF_ADDR, &invader_buf, FRAME_WIDTH*(y_pos+220-(48*dead_rows))*sizeof(uint16_t));
}

/*
checks if and which alien is hit
*/
void check_hit(struct position *shooter_pos)
{
    //check which invader position in the array is shot
    int xdiff = (shooter_pos->x_pos+45)-(invaders_position.x_pos+24);
    int xpos = xdiff /48;
    int ydiff = (shooter_pos -> y_pos)-(invaders_position.y_pos);
    int ypos = ydiff /48;

    if(xpos >= 0  &&  xpos <= 11 && ydiff >= 0 && ypos<= 4){

        if(invaders_s[xpos][ypos].alive == true)
        {
            invaders_s[xpos][ypos].alive = false;
            copy_bitmap((uint16_t*)VNC_FRAME_BUF_ADDR, &projectile_black, shooter_pos -> x_pos, shooter_pos -> y_pos );
            print_invaders(invaders_position.x_pos,invaders_position.y_pos,0);
            shooter_pos -> x_pos = -1;
        }
    }
}
/**
checks if the end is reached
*/
bool check_gameover()
{
    if( invaders_position.y_pos > 160 +  (dead_rows*48)) return true;
    else return false;
}

/*
projectile structure:
there can be 5 projectiles at the same time printed on the screen.
the projectiles are stored in an array of positions.
If the x value of a position is <-1, the projectile isn't active(not printed)
*/

void add_projectile(int xpos, int ypos)
{
 for(int i = 0; i<5; i++)
 {
    if(projectile_positions[i].x_pos == -1)
    {
        projectile_positions[i].x_pos = xpos;
        projectile_positions[i].y_pos = ypos;
        break;
    }
 }
}

/*update projectile position*/
void update_projectile()
{
 for(int i = 0; i<5; i++)
 {
    if(projectile_positions[i].x_pos != -1) //check if projectile is active
    {
        if(projectile_positions[i].y_pos < 10) //check if projectile is out of screen
        {
            projectile_positions[i].x_pos = -1;
        }
        else
        {
            copy_bitmap((uint16_t*)VNC_FRAME_BUF_ADDR, &projectile, projectile_positions[i].x_pos, projectile_positions[i].y_pos );
            check_hit(&projectile_positions[i]);
            projectile_positions[i].y_pos -= 2;
        }
    }
 }
}

/*
    Set init values
*/
void init()
{
    //initilize invaders
    static Bitmap *bitmap_ptrs[5][2]= {{&Alien1_1,&Alien1_2},{&Alien2_1,&Alien2_2},{&Alien2_1,&Alien2_2},{&Alien3_1,&Alien3_2},{&Alien3_1,&Alien3_2}}; // create an array of pointer to the bitmaps
    for(int j = 0; j< 5; j++)
    {
        for(int i = 0; i<11; i++)
        {
            invaders_s[i][j].alive = true;
            invaders_s[i][j].bitmap[0] = bitmap_ptrs[j][0];
            invaders_s[i][j].bitmap[1] = bitmap_ptrs[j][1];
        }
    }

    //initialize projectiles
     for(int i = 0; i<5; i++)
    {
            projectile_positions[i].x_pos = -1;
            projectile_positions[i].y_pos = 0;
    }
    //print bunkers
    for(int i = 0; i<4; i++)
    {
        copy_bitmap ((uint16_t*)VNC_FRAME_BUF_ADDR, &Bunker, 102+i*174, 352);
    }
    invaders_position.x_pos = 10;
    invaders_position.y_pos = 10;
}


int main() {
    uint32_t kbd_key_reg;
    uint32_t key;

    uint64_t time_invaders = 0; 
    uint64_t time_projectile = 0;
    uint64_t stop_time = 0; 

    *VNC_KBD_CTRL_ADDR = *VNC_KBD_CTRL_ADDR | (1 << 0);
    kbd_key_reg = *VNC_KBD_KEY_ADDR;

    init();
    time_invaders = *mtime;
    time_projectile = *mtime;

    int x_shooter_pos = 20;
    bool game_over = false;
    enum Direction direction = RIGHT;

    copy_bitmap(VNC_FRAME_BUF_ADDR, &shooter, x_shooter_pos, 420);
    while(1)
    {   
        //move the invaders:
        if(*mtime - time_invaders >= 500000) { //wait until update invaders
            time_invaders = *mtime;
            if(invaders_position.x_pos%10 == 0)
                print_invaders(invaders_position.x_pos,invaders_position.y_pos,0);
            else
                print_invaders(invaders_position.x_pos,invaders_position.y_pos,1);

            if(direction == RIGHT) 
                invaders_position.x_pos+=5;
            else
                invaders_position.x_pos-=5;

            if (invaders_position.x_pos >= 240 || invaders_position.x_pos <= 10)
            {
                check_dead_rows();
                if(check_gameover())
                {
                    break; //end game
                }
                else
                {
                    //now move invaders to the opposite direction
                    switch(direction)
                    {
                        case RIGHT:
                            direction = LEFT;
                            break;
                        case LEFT:
                            direction = RIGHT;
                            break;
                    }
                    invaders_position.y_pos += 10;
                }
            }
        }

        //move the shooter:
        kbd_key_reg = *VNC_KBD_KEY_ADDR;

        if(*mtime - time_projectile >= 10000) { // wait until update projectiles
            time_projectile = *mtime;
            update_projectile();
        }


        if(kbd_key_reg & (1 << 31)) {
            key = (kbd_key_reg & (0x7FFFFFFE))>>1;
            if(kbd_key_reg & (1 << 0))
            {
                switch(key)
                {
                    case 32: //move shooter to the right with D
                        x_shooter_pos += 10; 
                        break;
                    case 30: //move shooter to the left with AA
                        x_shooter_pos -= 10;
                        break;
                    case 57: //shoot with space
                        add_projectile(x_shooter_pos+41, 392);
                        break;
                }
                copy_bitmap(VNC_FRAME_BUF_ADDR, &shooter, x_shooter_pos, 420); //update shooter position
            }
        }
    }

    //game over
    memcpy(VNC_FRAME_BUF_ADDR, &BACKGROUND, FRAME_BUF_SIZE);
    copy_bitmap(VNC_FRAME_BUF_ADDR, &game_over_bmp, 0, 0);
    while(1);
    
}