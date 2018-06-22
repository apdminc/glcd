/**
   \file glcd.h
   \brief GLCD Library main header file. This file must be included into project.
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

#ifndef _GLCD_H
#define _GLCD_H

#define GLCD_DEVICE_STM32F4XX_CHIBIOS
#define GLCD_CONTROLLER_SHARP_LS013B7DH03
#define GLCD_USE_SPI_UART             0
#define CHIBIOS_SPI_PEREPHERIAL       &SPID5
#define CHIBIOS_UART_SPI_PEREPHERIAL  &UARTD3


#define GLCD_LSB_STRAIGT_BUFFER_PACKING   1
#define GLCD_MSB_BUFFER_PACKING       0


#if defined(GLCD_DEVICE_STM32F0XX)
	#include <stm32f0xx.h>
	#include <stm32f0xx_gpio.h>
	#include "devices/inc/STM32F0xx.h"
	extern void delay_ms(uint32_t ms);
	#define PROGMEM
	
#elif defined(GLCD_DEVICE_STM32F4XX_CHIBIOS)
	//Allows you to use a UART peripheral as a SPI bus (STM32 can do this).

    #define GLCD_USE_CORTEX_M3_INSTRUCTIONS TRUE

#if GLCD_USE_SPI_UART
    #ifndef CHIBIOS_UART_SPI_PEREPHERIAL
        #error "You must define CHIBIOS_UART_SPI_PEREPHERIAL somewhere, E.G. #define CHIBIOS_UART_SPI_PEREPHERIAL &UARTD3"
    #endif
#else
    #ifndef CHIBIOS_SPI_PEREPHERIAL
        #error "You must define CHIBIOS_SPI_PEREPHERIAL somewhere, E.G. #define CHIBIOS_SPI_PEREPHERIAL &SPID1"
    #endif
#endif


    #define delay_ms(t)                   chThdSleepMilliseconds(t)
    #include "devices/STM32F4_ChibiOS.h"
    #define PROGMEM

#else
	#error "Device not supported"
	
#endif
	
#if defined(GLCD_CONTROLLER_SHARP_LS013B7DH03)
	/* Note: you must either externally toggle the VCOM pin at at least 1hz, or configure the chip
	 * and toggle the vcom bit in the SPI payload. This code assumes external toggling.
	 */
    #include "controllers/sharp_LS013B7DH03.h"
    #define   USE_SPI_MULTIBYTE
    #define   BLACK 0
    #define   WHITE 1

    #if defined(GLCD_USE_PARALLEL)
        #error "The SHARP_LS013B7DH03 must use SPI"
    #endif

	void glcd_write_bounded(const int ymin, const int ymax);

#else
	#error "Controller not supported"
	
#endif

#define swap(a, b) { uint8_t t = a; a = b; b = t; }


#include <stdint.h>
#include "glcd_devices.h"
#include "glcd_controllers.h"
#include "glcd_graphics.h"
#include "glcd_graphs.h"
#include "glcd_text_tiny.h"
#include "glcd_text.h"
#include "unit_tests.h"


/**@}*/

/**
 * \name LCD Dimensions
 * @{
 */

/* Automatic assignment of width and height, if required. */
#if defined(GLCD_CONTROLLER_SHARP_LS013B7DH03)
    #define GLCD_LCD_WIDTH  128
    #define GLCD_LCD_HEIGHT 128
#endif

/*
 * GLCD_NUMBER_OF_BANKS is typically GLCD_LCD_HEIGHT/8
 * Don't adjust these below unless required.
 */
#define GLCD_NUMBER_OF_BANKS (GLCD_LCD_WIDTH / 8)
#define GLCD_NUMBER_OF_COLS  GLCD_LCD_WIDTH

/**@}*/

#if !defined(GLCD_RESET_TIME)
/** Reset duration by glcd_reset(), in milliseconds */
#define GLCD_RESET_TIME 1
#endif


#define MODIFIED_ROWS_SIZE ((GLCD_LCD_WIDTH / 8) + 1)
/**
 * Bounding box for pixels that need to be updated
 */
typedef struct {
	uint8_t x_min;
	uint8_t y_min;
	uint8_t x_max;
	uint8_t y_max;

	uint8_t modified_rows_bitmask[MODIFIED_ROWS_SIZE];
} glcd_BoundingBox_t;

typedef struct {
  uint32_t start_row_buffer_index;
  uint32_t buffer_index;
  uint8_t bitmask;
} buffer_packing_struct_t;

typedef enum {
  GLCD_SCREEN_ROTATION_0_DEGREES = 0,
  GLCD_SCREEN_ROTATION_90_DEGREES = 90,
  GLCD_SCREEN_ROTATION_180_DEGREES = 180,
  GLCD_SCREEN_ROTATION_270_DEGREES = 270
} glcd_screen_rotation_mode_t;

/* Global variables used for GLCD library */
extern uint8_t glcd_buffer[(GLCD_LCD_WIDTH * GLCD_LCD_HEIGHT / 8) + 1];
extern glcd_BoundingBox_t glcd_bbox;
extern glcd_screen_rotation_mode_t glcd_screen_rotation;


/** \name Base Functions
 *  @{
 */

void glcd_get_buffer_pos(uint8_t x, uint8_t y, buffer_packing_struct_t *bps);

uint8_t glcd_get_foreground_color(void);
uint8_t glcd_get_background_color(void);
void glcd_set_foreground_color(uint8_t color);

/**
 * Update bounding box.
 *
 * The bounding box defines a rectangle in which needs to be refreshed next time
 * glcd_write() is called. glcd_write() only writes to those pixels inside the bounding box plus any
 * surrounding pixels which are required according to the bank/column write method of the controller.
 *
 * Define a rectangle here, and it will be <em>added</em> to the existing bounding box.
 *
 * \param xmin Minimum x value of rectangle
 * \param ymin Minimum y value of rectangle
 * \param xmax Maximum x value of rectangle
 * \param ymax Maximum y value of rectangle
 * \see glcd_bbox
 */
void glcd_update_bbox(uint8_t xmin, uint8_t ymin, uint8_t xmax, uint8_t ymax);

/**
 * Reset the bounding box.
 * After resetting the bounding box, no pixels are marked as needing refreshing.
 */
void glcd_reset_bbox(void);

/**
 * Marks the entire display for re-writing.
 */
void glcd_bbox_refresh(void);
	
/**
 * Clear the display. This will clear the buffer and physically write and commit it to the LCD
 */
void glcd_clear(void);

/**
 * Clear the display buffer only. This does not physically write the changes to the LCD
 */
void glcd_clear_buffer(void);


void glcd_set_screen_rotation(const glcd_screen_rotation_mode_t mode);
glcd_screen_rotation_mode_t glcd_get_screen_rotation(void);

/**
 * FIXME document this
 */
uint8_t glcd_reverse_significant_bits(uint32_t value);

/** @}*/
enum font_table_type { STANG, MIKRO };

typedef struct {
	const char *font_table;
	uint8_t width;
	uint8_t height;
	char start_char;
	char end_char;
	enum font_table_type table_type;
} glcd_FontConfig_t;

extern glcd_FontConfig_t font_current;

#endif
