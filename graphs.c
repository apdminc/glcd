/**
   \file graphs.c
   \brief Functions relating to graphs. e.g bar graphs etc.
   \author Andy Gock
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

#include "glcd.h"

uint8_t graph_color_line = WHITE;
uint8_t graph_color_background = BLACK;



static uint8_t glcd_map(uint8_t x1, uint8_t x2, uint8_t x);

void glcd_graph_set_foreground_color(uint8_t color) {
  graph_color_line = color;
  graph_color_background = (color == BLACK ? WHITE : BLACK);
}

void glcd_bar_graph_horizontal(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t val)
{
	if (height < 3) {
		return;
	}
	glcd_draw_rect(x, y, width, height, graph_color_line);
	glcd_fill_rect(x+1, y+1, glcd_map(0,width-2,val), height-2 , graph_color_line);
}

void glcd_bar_graph_horizontal_no_border(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t val)
{
	if (height < 3) {
		return;
	}	
	glcd_fill_rect(x, y, glcd_map(0,width,val), height , graph_color_line);
}

void glcd_bar_graph_vertical(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t val)
{
	glcd_draw_rect(x, y, width, height, graph_color_line);
	glcd_fill_rect(x+1, y+1, width-2, y+1+glcd_map(0,height-3,255-val), graph_color_background);
	glcd_fill_rect(x+1, y+1+glcd_map(0,height-2,255-val), width-2, height-2-glcd_map(0,height-2,255-val), graph_color_line);
}

void glcd_bar_graph_vertical_no_border(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t val)
{
	glcd_fill_rect(x, y+glcd_map(0,height-2,255-val), width, height-2-glcd_map(0,height-2,255-val), graph_color_line);
}

void glcd_scrolling_bar_graph(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t val)
{
	uint8_t nx, ny;
	uint8_t color;
	
	/* Draw border of graph */
	glcd_draw_rect(x,y,width,height,graph_color_line);
	
	/* Scroll inner contents left by one pixel width */
	for (ny = 1; ny <= (height-2); ny++) {
		/* Redraw each horizontal line */
		for (nx = 1; nx <= (width-2); nx += 1) {
			color = glcd_get_pixel(x+nx+1,y+ny);
			glcd_set_pixel(x+nx,y+ny,color);
		}
	}
	
	val = val * (height-3) / 255;
	
	/* Make sure we're not exceeding the size of box interior */
	if (val > (height-3)) {
		val = height - 3;
	}
	
	/* Draw new bar - both black and white portions*/
	glcd_draw_line(x+width-2,y+height-2,x+width-2,y+height-2-val,graph_color_line);
	glcd_draw_line(x+width-2,y+height-3-val,x+width-2,y+1,graph_color_background);
	
	/* Write to display */
	//glcd_write();
}

void glcd_scrolling_line_graph(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t left_step_count, uint8_t val, uint8_t x_position_subtractor)
{
    /* Scroll inner contents left by left_step_count pixels width */
    if( left_step_count > 0 ) {
      /* Draw border of graph */
      glcd_draw_rect(x,y,width,height,graph_color_line);

      for (uint8_t ny = 1; ny <= (height-2); ny++) {
          /* Redraw each horizontal line */
          for (uint8_t nx =  1; nx <= (width-2-left_step_count-1); nx += 1) {
              uint8_t color = glcd_get_pixel(x+nx+left_step_count-1, y+ny);
              glcd_set_pixel(x+nx, y+ny, color);
          }
      }

      /* Draw new bar - both black and white portions*/
      glcd_draw_line(x+width-2,y+height-2,x+width-2,y+1,graph_color_background);
    }

    val = val * (height-3) / 255;

    /* Make sure we're not exceeding the size of box interior */
    if (val > (height-3)) {
        val = height - 3;
    }

    //glcd_draw_line(x+width-2, y+height-2,x+width-2,y+height-2-val,graph_color_background);
    const uint8_t x_pos = x + width - 2 - x_position_subtractor;
    glcd_draw_line(x_pos, y+height-2, x_pos, y+1, graph_color_background);//black out the background
    glcd_set_pixel(x_pos, y+height-2-val, graph_color_line);//place a pixel on top of what you blacked out


    /* Write to display */
    //glcd_write();
}

void glcd_scrolling_bar_graph_timing(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t val, uint8_t line_width, uint16_t delay)
{
	uint8_t n;
	if (line_width == 0) {
		line_width = 1;
	}
	
	/* Adjust graph line's width by just running glcd_scrolling_bar_graph() x number of times */
	/* \todo This should be done differently! */
	for (n=0; n<line_width; n++) {
		glcd_scrolling_bar_graph(x,y,width,height,val);
	}
	
	if (delay) {
		delay_ms(delay);
	}
}

static uint8_t glcd_map(uint8_t x1, uint8_t x2, uint8_t x)
{
	return x1+(x2-x1)*x/255;	
}
