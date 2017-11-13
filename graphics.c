/**
   \file graphics.c
   \brief Functions relating to graphics. e.g drawing lines, rectangles, circles etc.
   \author Andy Gock

   Some functions based on Limor Fried's PCD8544 Arduino library.

 */ 

/*
	Copyright (c) 2012, Andy Gock

	Copyright (c) 2012, Adafruit Industries

	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:
		* Redistributions of source code must retain the above copyright
		  notice, this list of conditions and the following disclaimer.
		* Redistributions in binary form must reproduce the above copyright
		  notice, this list of conditions and the following disclaimer in the
		  documentation and/or other materials provided with the distribution.
		* Neither the name of Andy Gock nor the
		  names of its contributors may be used to endorse or promote products
		  derived from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL ANDY GOCK BE LIABLE FOR ANY
	DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
	ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "glcd.h"

#if GLCD_MSB_BUFFER_PACKING
#  define GLCD_BUFFER_BYTE_MASK(y) (1 << (y % 8))
#else
#  define GLCD_BUFFER_BYTE_MASK(y) (0x80 >> (y % 8))
#endif

void glcd_set_screen_rotation(const glcd_screen_rotation_mode_t mode) {
  glcd_screen_rotation = mode;
}

glcd_screen_rotation_mode_t glcd_get_screen_rotation(void) {
  return(glcd_screen_rotation);
}

typedef struct {
  //uint8_t start_row_buffer_index;
  uint32_t buffer_index;
  uint8_t bitmask;
} buffer_packing_struct_t;

void glcd_get_buffer_pos(uint8_t x, uint8_t y, buffer_packing_struct_t *bps) {
  //bps->start_row_buffer_index = 0;
  bps->buffer_index = ((y/8)*GLCD_LCD_WIDTH) + x;
  bps->bitmask = GLCD_BUFFER_BYTE_MASK(y);
}

static void glcd_rotate_pixels(uint8_t *x, uint8_t *y) {
  const uint8_t x_orig = *x;
  const uint8_t y_orig = *y;

  switch(glcd_screen_rotation) {
    case GLCD_SCREEN_ROTATION_0_DEGREES:
      break;
    case GLCD_SCREEN_ROTATION_90_DEGREES:
      *x = y_orig;
      *y = GLCD_LCD_HEIGHT - 1 - x_orig;
      break;
    case GLCD_SCREEN_ROTATION_180_DEGREES:
      *x = GLCD_LCD_WIDTH - 1 - x_orig;
      *y = GLCD_LCD_HEIGHT - 1 - y_orig;
      break;
    case GLCD_SCREEN_ROTATION_270_DEGREES:
      *x = GLCD_LCD_WIDTH - 1 - y_orig;
      *y = x_orig;
      break;
  }
}

void glcd_set_screen_buffer(const uint8_t color) {
  if( color ) {
    memset(glcd_buffer, 0xFF, sizeof(glcd_buffer));
  } else {
    memset(glcd_buffer, 0x00, sizeof(glcd_buffer));
  }
  glcd_update_bbox(0,0,(GLCD_LCD_WIDTH-1),(GLCD_LCD_HEIGHT-1));
}

/* Based on PCD8544 library by Limor Fried */
void glcd_set_pixel(uint8_t x, uint8_t y, const uint8_t color) {
    glcd_rotate_pixels(&x, &y);
	if (x > (GLCD_LCD_WIDTH-1) || y > (GLCD_LCD_HEIGHT-1)) {
		/* don't do anything if x/y is outside bounds of display size */
		return;
	}

	buffer_packing_struct_t bps;
	glcd_get_buffer_pos(x, y, &bps);

	//const uint32_t buffer_index = ((y/8) * GLCD_LCD_WIDTH) + x;
	const uint8_t old_value = glcd_buffer[bps.buffer_index];
	if (color) {
		/* Set black */
		glcd_buffer[bps.buffer_index] |= bps.bitmask;
	} else {
		/* Set white */
		glcd_buffer[bps.buffer_index] &= ~(bps.bitmask);
	}

	if( old_value != glcd_buffer[bps.buffer_index] ) {
	  glcd_update_bbox(x,y,x,y);
	}
}



/* Based on PCD8544 library by Limor Fried */
uint8_t glcd_get_pixel(uint8_t x, uint8_t y) {
    glcd_rotate_pixels(&x, &y);
	if ((x >= GLCD_LCD_WIDTH) || (y >= GLCD_LCD_HEIGHT)) {
		return 0;
	}
	
    buffer_packing_struct_t bps;
    glcd_get_buffer_pos(x, y, &bps);

	if ( glcd_buffer[bps.buffer_index]  & (bps.bitmask) ) {
		return 1;
	} else {
		return 0;
	}
}

void glcd_invert_pixel(uint8_t x, uint8_t y) {
    glcd_rotate_pixels(&x, &y);
	if ((x >= GLCD_LCD_WIDTH) || (y >= GLCD_LCD_HEIGHT)) {
		return;
	}

	buffer_packing_struct_t bps;
	glcd_get_buffer_pos(x, y, &bps);

	glcd_buffer[bps.buffer_index] ^= (bps.bitmask);
	glcd_update_bbox(x,y,x,y);
}

/* Bresenham's algorithm - based on PCD8544 library Limor Fried */
void glcd_draw_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t color) {
	uint8_t steep = abs(y1 - y0) > abs(x1 - x0);
	uint8_t dx, dy;
	int8_t err;
	int8_t ystep;
	
	if (steep) {
		swap(x0, y0);
		swap(x1, y1);
	}
	
	if (x0 > x1) {
		swap(x0, x1);
		swap(y0, y1);
	}
	
	glcd_update_bbox( x0, y0, x1, y1 );

	dx = x1 - x0;
	dy = abs(y1 - y0);
	
	err = dx / 2;
	
	if (y0 < y1) {
		ystep = 1;
	} else {
		ystep = -1;
	}
	
	for (; x0<=x1; x0++) {
		if (steep) {
			glcd_set_pixel(y0, x0, color);
		} else {
			glcd_set_pixel(x0, y0, color);
		}
		err -= dy;
		if (err < 0) {
			y0 += ystep;
			err += dx;
		}
	}
}

void glcd_fill_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color)
{
	int16_t i;
	for (i=x; i<x+w; i++) {
		int16_t j;
		for (j=y; j<y+h; j++) {
			glcd_set_pixel(i, j, color);
		}
	}
	glcd_update_bbox(x, y, x+w-1, y+h-1);
}

void glcd_draw_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color)
{
	int16_t i;
	for (i=x; i<x+w; i++) {
		glcd_set_pixel(i, y, color);
		glcd_set_pixel(i, y+h-1, color);
	}
	for (i=y; i<y+h; i++) {
		glcd_set_pixel(x, i, color);
		glcd_set_pixel(x+w-1, i, color);
	} 
	glcd_update_bbox(x, y, x+w-1, y+h-1);
}

void glcd_draw_rect_thick(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t tx, uint8_t ty, uint8_t color)
{
	int16_t i, t;
	
	if (tx == 0) {
		tx = 1;
	}

	if (ty == 0) {
		ty = 1;
	}
	
	for (i=x; i<x+w; i++) {
		/* Top and bottom sides */
		for (t=0; t<(ty); t++) {
			glcd_set_pixel(i, y+t, color);
			glcd_set_pixel(i, y+h-1-t, color);
		}
	}
	for (i=y; i<y+h; i++) {
		/* Left and right sides */
		for (t=0; t<(tx); t++) {
			glcd_set_pixel(x+t, i, color);
			glcd_set_pixel(x+w-1-t, i, color);
		}
	} 
	glcd_update_bbox(x, y, x+w-1, y+h-1);
}

void glcd_draw_rect_shadow(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color)
{
	glcd_draw_rect(x, y, w, h, color);
	glcd_draw_line(x+1, y+h, x+w, y+h, color);
	glcd_draw_line(x+w, y+1, x+w, y+h, color);
}


void glcd_set_pixel_a(const uint8_t x, const uint8_t y, const uint8_t color, const uint8_t min_x, const uint8_t min_y, const uint8_t max_x, const uint8_t max_y)
{
  if( x >= min_x && x <= max_x && y >= min_y && y <= max_y ) {
    glcd_set_pixel(x, y, color);
  }
}


void glcd_draw_circle_section(const int16_t x0, const int16_t y0, const uint16_t r, const uint8_t color, const uint8_t min_x, const uint8_t min_y, const uint8_t max_x, const uint8_t max_y)
{
    int8_t f = 1 - r;
    int8_t ddF_x = 1;
    int8_t ddF_y = -2 * r;
    int8_t x = 0;
    int8_t y = r;

    glcd_update_bbox(x0-r, y0-r, x0+r, y0+r);

    glcd_set_pixel_a(x0, y0+r, color, min_x, min_y, max_x, max_y);
    glcd_set_pixel_a(x0, y0-r, color, min_x, min_y, max_x, max_y);
    glcd_set_pixel_a(x0+r, y0, color, min_x, min_y, max_x, max_y);
    glcd_set_pixel_a(x0-r, y0, color, min_x, min_y, max_x, max_y);

    while (x<y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        glcd_set_pixel_a(x0 + x, y0 + y, color, min_x, min_y, max_x, max_y);
        glcd_set_pixel_a(x0 - x, y0 + y, color, min_x, min_y, max_x, max_y);
        glcd_set_pixel_a(x0 + x, y0 - y, color, min_x, min_y, max_x, max_y);
        glcd_set_pixel_a(x0 - x, y0 - y, color, min_x, min_y, max_x, max_y);

        glcd_set_pixel_a(x0 + y, y0 + x, color, min_x, min_y, max_x, max_y);
        glcd_set_pixel_a(x0 - y, y0 + x, color, min_x, min_y, max_x, max_y);
        glcd_set_pixel_a(x0 + y, y0 - x, color, min_x, min_y, max_x, max_y);
        glcd_set_pixel_a(x0 - y, y0 - x, color, min_x, min_y, max_x, max_y);

    }
}

void glcd_draw_circle(uint8_t x0, uint8_t y0, uint8_t r, uint8_t color)
{
		
	int8_t f = 1 - r;
	int8_t ddF_x = 1;
	int8_t ddF_y = -2 * r;
	int8_t x = 0;
	int8_t y = r;
	
	glcd_update_bbox(x0-r, y0-r, x0+r, y0+r);
	
	glcd_set_pixel(x0, y0+r, color);
	glcd_set_pixel(x0, y0-r, color);
	glcd_set_pixel(x0+r, y0, color);
	glcd_set_pixel(x0-r, y0, color);
	
	while (x<y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;
		
		glcd_set_pixel(x0 + x, y0 + y, color);
		glcd_set_pixel(x0 - x, y0 + y, color);
		glcd_set_pixel(x0 + x, y0 - y, color);
		glcd_set_pixel(x0 - x, y0 - y, color);
		
		glcd_set_pixel(x0 + y, y0 + x, color);
		glcd_set_pixel(x0 - y, y0 + x, color);
		glcd_set_pixel(x0 + y, y0 - x, color);
		glcd_set_pixel(x0 - y, y0 - x, color);
		
	}
}

void glcd_fill_circle_section(uint8_t x0, uint8_t y0, uint8_t r, uint8_t color, const uint8_t min_x, const uint8_t min_y, const uint8_t max_x, const uint8_t max_y)
{

    int8_t f = 1 - r;
    int8_t ddF_x = 1;
    int8_t ddF_y = -2 * r;
    int8_t x = 0;
    int8_t y = r;

    int16_t i;

    glcd_update_bbox(x0-r, y0-r, x0+r, y0+r);

    for (i=y0-r; i<=y0+r; i++) {
        glcd_set_pixel_a(x0, i, color, min_x, min_y, max_x, max_y);
    }

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        for (i=y0-y; i<=y0+y; i++) {
            glcd_set_pixel_a(x0+x, i, color, min_x, min_y, max_x, max_y);
            glcd_set_pixel_a(x0-x, i, color, min_x, min_y, max_x, max_y);
        }
        for (i=y0-x; i<=y0+x; i++) {
            glcd_set_pixel_a(x0+y, i, color, min_x, min_y, max_x, max_y);
            glcd_set_pixel_a(x0-y, i, color, min_x, min_y, max_x, max_y);
        }
    }
}

void glcd_fill_circle(uint8_t x0, uint8_t y0, uint8_t r, uint8_t color)
{
	
	int8_t f = 1 - r;
	int8_t ddF_x = 1;
	int8_t ddF_y = -2 * r;
	int8_t x = 0;
	int8_t y = r;
	
	int16_t i;

	glcd_update_bbox(x0-r, y0-r, x0+r, y0+r);
	
	for (i=y0-r; i<=y0+r; i++) {
		glcd_set_pixel(x0, i, color);
	}
	
	while (x < y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;
		
		for (i=y0-y; i<=y0+y; i++) {
			glcd_set_pixel(x0+x, i, color);
			glcd_set_pixel(x0-x, i, color);
		} 
		for (i=y0-x; i<=y0+x; i++) {
			glcd_set_pixel(x0+y, i, color);
			glcd_set_pixel(x0-y, i, color);
		}    
	}
}

void glcd_invert_area(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	uint8_t xx, yy;
	for (xx = x; xx < (x+w); xx++) {
		/* Loop through each partial column */
		for (yy = y; yy < (y+h); yy++) {
			/* Go down and invert every pixel */
			glcd_invert_pixel(xx,yy);
		}
	}
}

void glcd_draw_bitmap(const unsigned char *data)
{
	/* Copy bitmap data to the screen buffer */
	memcpy(glcd_buffer, data, (GLCD_LCD_WIDTH * GLCD_LCD_HEIGHT / 8));

	glcd_bbox_refresh(); 
}
