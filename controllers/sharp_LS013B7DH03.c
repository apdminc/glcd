#include "../glcd.h"

#if defined(GLCD_CONTROLLER_SHARP_LS013B7DH03)

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

static uint8_t msb_to_lsb(uint8_t v)
{
  //this assumes that the SPI peripheral is in LSB mode
  return(v);
}

bool_t sharp_lcd_write_line(const uint8_t line_number, const uint8_t *buff)
{
  if( line_number < 1 || line_number > MLCD_YRES ) {
    return(false);
  }

  static uint8_t cmd_buff[1 + 1 + MLCD_BYTES_LINE + 2];
  cmd_buff[0] = MLCD_WR_MSB;
  cmd_buff[1] = msb_to_lsb(line_number);
  for(int i = 0; i < MLCD_BYTES_LINE; i++ ) {
    cmd_buff[2 + i] = msb_to_lsb(buff[i]);
  }
  cmd_buff[MLCD_BYTES_LINE + 2] = 0;//trailer
  cmd_buff[MLCD_BYTES_LINE + 3] = 0;//trailer

  GLCD_DESELECT();
  glcd_spi_write_multibyte(sizeof(cmd_buff), cmd_buff);
  GLCD_SELECT();

  return(true);
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


/* Write screen buffer to display, within bounding box only */
void glcd_write()
{
    //FIXME do bounding box logic
    for(int row = 0; row < GLCD_LCD_HEIGHT; row++ ) {
      uint8_t bank_number = (row / 8);
      uint8_t bitmask = (1 << (row%8));

      static uint8_t reorg_buff[MLCD_BYTES_LINE];
      for(int i = 0; i < MLCD_XRES; i++ ) {
          //FIXME this is HORRIBLY inefficient, but functional for the moment...
          uint8_t byte_offset = i / 8;
          uint8_t bit_shift = (i%8);
          if( glcd_get_pixel(i, row) ) {
            reorg_buff[byte_offset] |= (1<<bit_shift);
          } else {
            reorg_buff[byte_offset] &= ~(1<<bit_shift);
          }
      }

      sharp_lcd_write_line((row + 1), reorg_buff);
    }

    /* Display updated, we can reset the bounding box */
    glcd_reset_bbox();
}


#endif


