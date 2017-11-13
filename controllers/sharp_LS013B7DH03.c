#include "../glcd.h"

#if defined(GLCD_CONTROLLER_SHARP_LS013B7DH03)

#include "string.h"



void glcd_command(uint8_t c)
{
}

void glcd_data(uint8_t c)
{
}

void glcd_set_contrast(uint8_t val)
{
}

void glcd_power_down(void)
{
}

void glcd_power_up(void)
{
}

void glcd_set_y_address(uint8_t y)
{
}

void glcd_set_x_address(uint8_t x)
{
}


static uint8_t to_lsb(uint8_t v)
{
#if 1
  /*
   * This assumes that the SPI peripheral is in LSB mode.
   */
  return(v);
#else
  /*
   * This assumes that the SPI peripheral is in MSB mode.
   */
  return(glcd_reverse_significant_bits(v));
#endif
}

void sharp_lcd_clear_screen(void)
{
  static uint8_t cmd_buff[2];
  cmd_buff[0] = MLCD_CM_MSB;
  cmd_buff[1] = 0;

  GLCD_DESELECT();
  glcd_spi_write_multibyte(sizeof(cmd_buff), cmd_buff);
  GLCD_SELECT();
}

#define GLCD_DIRTY_ROW_WRITES    1
uint32_t row_write_count = 0;

void glcd_write_bounded(const int ymin, const int ymax)
{
  int y_row_max = MLCD_YRES - 1;
  int y_row = 0;

  if( ymin >= 0 ) {
    y_row = ymin;
  } else if( glcd_bbox_selected != NULL ) {
    y_row = glcd_bbox_selected->y_min;
  }

  if( ymax >= 0 ) {
    y_row_max = ymax;
  } else if( glcd_bbox_selected != NULL ) {
    y_row_max = glcd_bbox_selected->y_max;
  }

  //FIXME for some reason, when text is written the bounding box values are not updated properly
  y_row_max = MLCD_YRES - 1;
  y_row = 0;



  row_write_count = 0;
  for(; y_row <= y_row_max && y_row >= 0 && y_row < MLCD_YRES; y_row++ ) {
#if GLCD_DIRTY_ROW_WRITES
    if( glcd_bbox_selected != NULL ) {
      const int byte_offset = y_row / 8;
      const int bit_offset = y_row % 8;
      if( ! (glcd_bbox_selected->modified_rows_bitmask[byte_offset] & (1<<bit_offset)) ) {
        //Row dirty bit check
        continue;
      } else {
        row_write_count++;
      }
    }
#endif

    //[command:1 byte][line:1 byte][pixels: 128/8 bytes][trailer: 2 bytes, timing requirement] See datasheet for details
    static uint8_t cmd_buff[1 + 1 + MLCD_BYTES_LINE + 2];
    memset(cmd_buff, 0, sizeof(cmd_buff));
    cmd_buff[0] = MLCD_WR_MSB;

    for(int i = 0; i < (GLCD_LCD_HEIGHT/8); i++ ) {
#if GLCD_MSB_BUFFER_PACKING
      cmd_buff[2 + ((GLCD_LCD_HEIGHT/8) - 1 - i)] = glcd_reverse_significant_bits(glcd_buffer[y_row + (i * GLCD_LCD_WIDTH)]);
#else
      cmd_buff[2 + ((GLCD_LCD_HEIGHT/8) - 1 - i)] = glcd_buffer[y_row + (i * GLCD_LCD_WIDTH)];
#endif
    }

    cmd_buff[1] = to_lsb(y_row + 1);//lines are 1-based on the LCD itself

    GLCD_DESELECT();
    glcd_spi_write_multibyte(sizeof(cmd_buff), cmd_buff);
    GLCD_SELECT();

#if 0
    chThdSleepMicroseconds(6);
#else
    for(int i = 0; i < 500; i++ ) {
      //This loop need to burn about 6 micro seconds to satisfy the sharp LCD Minimum SCS low time
      __NOP();
    }
#endif
  }


  /* Display updated, we can reset the bounding box */
  glcd_reset_bbox();
}


/* Write screen buffer to display, within bounding box only */
void glcd_write() {
  glcd_write_bounded(-1, -1);
}


#endif


