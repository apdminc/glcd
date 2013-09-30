#include "../glcd.h"

#if defined(GLCD_DEVICE_STM32F4XX_CHIBIOS)

void glcd_init(void)
{
#if defined(GLCD_CONTROLLER_SHARP_LS013B7DH03)
    //Note: SPI pins and controller should be initialized by chibios, using a combonation of main() and the board.h file.
#else
    #error "Controller not supported."
#endif
}

#ifdef USE_SPI_MULTIBYTE

void glcd_spi_write_multibyte(const uint16_t length, const uint8_t *source_buffer)
{
#if GLCD_USE_SPI_UART
  if( (CHIBIOS_UART_SPI_PEREPHERIAL)->state == UART_READY ) {
    uartStartSend(CHIBIOS_UART_SPI_PEREPHERIAL, length, source_buffer);
    spi_uart_wait_callback();
  }

#else
  spiSend(CHIBIOS_SPI_PEREPHERIAL, length, source_buffer);//chibios function call
#endif
}

#endif


#endif
