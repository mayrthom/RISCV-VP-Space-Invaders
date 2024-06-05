/*
 * Copyright (C) 2024 Thomas Mayr <mayr.t@aon.at>
 * Copyright (C) 2024 Johannes Artelsmair <johannes.artelsmair@gmx.at>
 *
 * SPDX-License-Identifier: MIT
 */


#ifndef GRAPHICS_H

#define GRAPHICS_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#define FRAME_WIDTH 800
#define FRAME_HEIGHT 480
#define FRAME_BUF_SIZE (FRAME_WIDTH * FRAME_HEIGHT * 2)

struct bitmap {
        int width;
        int height;
        uint16_t data[];
};

typedef struct bitmap Bitmap;


void copy_bitmap(uint16_t *framebuf, Bitmap* b, int x_pos, int y_pos)
{
    for(int i = 0; i<( b -> height); i++)
    {
        memcpy( framebuf + y_pos*FRAME_WIDTH + (x_pos+i*FRAME_WIDTH), &((b -> data)[(b-> width) *i]), (b-> width)*sizeof(uint16_t));
    }

}

#endif /* GRAPHICS_H */