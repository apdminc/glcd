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


  for(; y_row <= y_row_max && y_row >= 0 && y_row < MLCD_YRES; y_row++ ) {
    static uint8_t cmd_buff[1 + 1 + MLCD_BYTES_LINE + 2];
    memset(cmd_buff, 0, sizeof(cmd_buff));
    cmd_buff[0] = MLCD_WR_MSB;
    uint8_t line_number = y_row;

    for(int i = 0; i < (GLCD_LCD_HEIGHT/8); i++ ) {
      cmd_buff[2 + ((GLCD_LCD_HEIGHT/8) - 1 - i)] = glcd_reverse_significant_bits(glcd_buffer[y_row + (i * GLCD_LCD_WIDTH)]);
    }

    cmd_buff[1] = to_lsb(line_number + 1);//lines are 1-based on the LCD itself

    GLCD_DESELECT();
    glcd_spi_write_multibyte(sizeof(cmd_buff), cmd_buff);
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


