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
  extern volatile uint32_t spi_uart_end_transmisson_flag;
  if( (CHIBIOS_UART_SPI_PEREPHERIAL)->state == UART_READY ) {
    //palSetPad(GPIOA, GPIOA_SPI1_NSS_LCD);
    spi_uart_end_transmisson_flag = 0;
    uartStartSend(CHIBIOS_UART_SPI_PEREPHERIAL, length, source_buffer);
    while( ! spi_uart_end_transmisson_flag ) {
      __NOP();
    }
    //chThdSleepMilliseconds(1);
    //palClearPad(GPIOA, GPIOA_SPI1_NSS_LCD);

#if 0
    USART_TypeDef *u = (CHIBIOS_UART_SPI_PEREPHERIAL)->usart;

    while((CHIBIOS_UART_SPI_PEREPHERIAL)->state == UART_TX_ACTIVE || ! (u->SR & USART_SR_TC) ) {
      //FIXME make this event driven so were not spinning and waiting. This takes about 190us for 16 bytes at 1.1mbit
      __NOP();
    }
#endif
  }

#else
  spiSend(CHIBIOS_SPI_PEREPHERIAL, length, source_buffer);//chibios function call
#endif
}

#endif


#endif
