#include "../glcd.h"

#if defined(GLCD_CONTROLLER_SHARP_LS013B7DH03)

#include "string.h"

#define SHARP_LCD_WRITE_ROW_COMMAND_LENGTH    (1 + 1 + MLCD_BYTES_LINE + 2)
static uint8_t sharp_lcd_cmd_buff[SHARP_LCD_WRITE_ROW_COMMAND_LENGTH];

void sharp_lcd_clear_screen(void)
{
  sharp_lcd_cmd_buff[0] = MLCD_CM_MSB;
  sharp_lcd_cmd_buff[1] = 0;

  GLCD_DESELECT();
  glcd_spi_write_multibyte(2, sharp_lcd_cmd_buff);
  GLCD_SELECT();
}

#define GLCD_DIRTY_ROW_WRITES    1
//uint32_t row_write_count = 0;

void glcd_write_bounded(const int ymin, const int ymax)
{
  int y_row_max = MLCD_YRES - 1;
  int y_row = 0;

  if( ymin >= 0 ) {
    y_row = ymin;
  } else {
    //FIXME for some reason, when text is written the bounding box values are not updated properly
    //y_row = glcd_bbox.y_min;
  }

  if( ymax >= 0 ) {
    y_row_max = ymax;
  } else {
    //FIXME for some reason, when text is written the bounding box values are not updated properly
    //y_row_max = glcd_bbox.y_max;
  }

  //[command:1 byte][line:1 byte][pixels: 128/8 bytes][trailer: 2 bytes, timing requirement] See datasheet for details

  memset(sharp_lcd_cmd_buff, 0, sizeof(sharp_lcd_cmd_buff));
  sharp_lcd_cmd_buff[0] = MLCD_WR_MSB;

  for(; y_row <= y_row_max && y_row >= 0 && y_row < MLCD_YRES; y_row++ ) {
#if GLCD_DIRTY_ROW_WRITES
    const int byte_offset = y_row / 8;
    const int bit_offset = y_row % 8;
    if( ! (glcd_bbox.modified_rows_bitmask[byte_offset] & (1<<bit_offset)) ) {
      //Row dirty bit check
      continue;
    }
#endif

#if GLCD_LSB_STRAIGT_BUFFER_PACKING
#if 1
    buffer_packing_struct_t bps;
    glcd_get_buffer_pos(0, y_row, &bps);
    memcpy(&sharp_lcd_cmd_buff[2], &glcd_buffer[bps.start_row_buffer_index], (GLCD_LCD_WIDTH/8));
#else
    for(uint8_t x_columns = 0; x_columns < GLCD_LCD_HEIGHT; x_columns += 8 ) {
      buffer_packing_struct_t bps;
      glcd_get_buffer_pos(x_columns, y_row, &bps);
      sharp_lcd_cmd_buff[2 + (x_columns/8)] = glcd_buffer[bps.buffer_index];
    }
#endif
#else
    for(int x_columns = 0; x_columns < (GLCD_LCD_HEIGHT/8); x_columns++ ) {
#if GLCD_MSB_BUFFER_PACKING
      sharp_lcd_cmd_buff[2 + ((GLCD_LCD_HEIGHT/8) - 1 - x_columns)] = glcd_reverse_significant_bits(glcd_buffer[y_row + (x_columns * GLCD_LCD_WIDTH)]);
#else
      sharp_lcd_cmd_buff[2 + ((GLCD_LCD_HEIGHT/8) - 1 - x_columns)] = glcd_buffer[y_row + (x_columns * GLCD_LCD_WIDTH)];
#endif
    }
#endif

    sharp_lcd_cmd_buff[1] = y_row + 1;//lines are 1-based on the LCD itself

    GLCD_DESELECT();
    glcd_spi_write_multibyte(SHARP_LCD_WRITE_ROW_COMMAND_LENGTH, sharp_lcd_cmd_buff);
    GLCD_SELECT();

    for(int i = 0; i < 500; i++ ) {
      //This loop need to burn about 6 micro seconds to satisfy the sharp LCD Minimum SCS low time
      __NOP();
    }
  }

  /* Display updated, we can reset the bounding box */
  glcd_reset_bbox();
}


/* Write screen buffer to display, within bounding box only */
void glcd_write() {
  glcd_write_bounded(-1, -1);
}


#endif


