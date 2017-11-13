/**
	\file glcd.c
	\author Andy Gock
	\brief Basic GLCD functions affecting bounding box manipulation,
	       clearing of screen and buffers, and basic scroll functions.
 */

/*
	Copyright (c) 2012, Andy Gock

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

#include <string.h>
#include <stdio.h>
#include "glcd.h"

/** \addtogroup GlobalVars Global Variables
 *  @{
 */

/**
 *  Screen buffer
 *
 *  Requires at least one bit for every pixel (e.g 504 bytes for 48x84 LCD)
 */
uint8_t glcd_buffer[(GLCD_LCD_WIDTH * GLCD_LCD_HEIGHT / 8) + 1];

/**
 * Keeps track of bounding box of area on LCD which need to be
 * updated next reresh cycle
 */
glcd_BoundingBox_t glcd_bbox;


/**
 * The current screen orientation
 */
glcd_screen_rotation_mode_t glcd_screen_rotation = GLCD_SCREEN_ROTATION_0_DEGREES;

uint8_t current_foreground_color = WHITE;
uint8_t current_background_color = BLACK;

uint8_t glcd_get_foreground_color(void) {
  return(current_foreground_color);
}

uint8_t glcd_get_background_color(void) {
  return(current_background_color);
}

void glcd_set_foreground_color(uint8_t color) {
  glcd_graph_set_foreground_color(color);
  glcd_text_set_foreground_color(color);

  current_foreground_color = color;
  current_background_color = (color == BLACK ? WHITE : BLACK);
}

/** @} */

void glcd_update_bbox(uint8_t xmin, uint8_t ymin, uint8_t xmax, uint8_t ymax)
{
	/* Keep and check bounding box within limits of LCD screen dimensions */
	if (xmin > (GLCD_LCD_WIDTH-1)) {
		xmin = GLCD_LCD_WIDTH-1;
	}
	if (xmax > (GLCD_LCD_WIDTH-1)) {
		xmax = GLCD_LCD_WIDTH-1;
	}

	if (ymin > (GLCD_LCD_HEIGHT-1)) {
		ymin = GLCD_LCD_HEIGHT-1;
	}
	if (ymax > (GLCD_LCD_HEIGHT-1)) {
		ymax = GLCD_LCD_HEIGHT-1;
	}

#if 0
    for(int v = ymin; v <= ymax; v++ ) {
      const uint32_t byte_offset = v / 8;
      const uint32_t bit_offset = v % 8;
      if( byte_offset < sizeof(glcd_bbox.modified_rows_bitmask) ) {
        glcd_bbox.modified_rows_bitmask[byte_offset] |= (1<<bit_offset);
      }
    }
#else
    for(int v = xmin; v <= xmax; v++ ) {
      const uint32_t byte_offset = v / 8;
      const uint32_t bit_offset = v % 8;
      if( byte_offset < sizeof(glcd_bbox.modified_rows_bitmask) ) {
        glcd_bbox.modified_rows_bitmask[byte_offset] |= (1<<bit_offset);
      }
    }
#endif

    /* Update the bounding box size */
    if (xmin < glcd_bbox.x_min) {
        glcd_bbox.x_min = xmin;
    }
    if (xmax > glcd_bbox.x_max) {
        glcd_bbox.x_max = xmax;
    }
    if (ymin < glcd_bbox.y_min) {
        glcd_bbox.y_min = ymin;
    }
    if (ymax > glcd_bbox.y_max) {
        glcd_bbox.y_max = ymax;
    }
}

void glcd_reset_bbox()
{
	/* Used after physically writing to the LCD */
	glcd_bbox.x_min = GLCD_LCD_WIDTH - 1;
	glcd_bbox.x_max = 0;
	glcd_bbox.y_min = GLCD_LCD_HEIGHT -1;
	glcd_bbox.y_max = 0;

	memset(&glcd_bbox.modified_rows_bitmask, 0x00, sizeof(glcd_bbox.modified_rows_bitmask));
}

void glcd_bbox_reset() {
	glcd_reset_bbox();
}

void glcd_bbox_refresh() {
	/* Marks bounding box as entire screen, so on next glcd_write(), it writes the entire buffer to the LCD */
	glcd_bbox.x_min = 0;
	glcd_bbox.x_max = GLCD_LCD_WIDTH - 1;
	glcd_bbox.y_min = 0;
	glcd_bbox.y_max = GLCD_LCD_HEIGHT -1;

	memset(&glcd_bbox.modified_rows_bitmask, 0xFF, sizeof(glcd_bbox.modified_rows_bitmask));
}



void glcd_clear_buffer(void) {
	memset(glcd_buffer, 0x00, sizeof(glcd_buffer));
	glcd_update_bbox(0,0,GLCD_LCD_WIDTH - 1,GLCD_LCD_HEIGHT - 1);
}

void glcd_clear(void) {
    glcd_clear_buffer();
    glcd_write();
}

#ifdef GLCD_USE_CORTEX_M3_INSTRUCTIONS
/*
 * 8-bit MSB->LSB or LSB->MSB conversions of bytes.
 */
inline __attribute__((always_inline))
uint8_t glcd_reverse_significant_bits(uint32_t value)
{
  //single cycle instruction to reverse the order of the bits
  uint32_t d;
  asm ("rbit  %[Rd], %[Rm]        \r\n"
       "lsrs  %[Rd], %[Rd], #24   \r\n"
       : [Rd] "=r" (d)
       : [Rm] "r" (value));
  return(d & 0xFF);
}

#else
/*
 * 8-bit MSB->LSB or LSB->MSB conversions of bytes.
 */
uint8_t glcd_reverse_significant_bits(uint32_t v) {
  uint8_t ret = (v & 0x01);

  for(int i = 1; i < 8; i++ ) {
    ret <<= 1;
    v >>= 1;
    if( v & 0x01 ) {
      ret |= 0x01;
    }
  }
  return(ret);
}
#endif

