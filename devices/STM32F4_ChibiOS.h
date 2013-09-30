#ifndef STM32F4_CHIBIOS_H_
#define STM32F4_CHIBIOS_H_

#include "ch.h"
#include "hal.h"

#if !defined(GLCD_USE_PARALLEL)
#if GLCD_USE_SPI_UART
    #define GLCD_SELECT()     palClearPad(GPIOA, GPIOA_SPI1_NSS_LCD)
    #define GLCD_DESELECT()   palSetPad(GPIOA, GPIOA_SPI1_NSS_LCD);
    //#define GLCD_SELECT()
    //#define GLCD_DESELECT()
#  else
    #define GLCD_SELECT()     spiSelect(CHIBIOS_SPI_PEREPHERIAL)
    #define GLCD_DESELECT()   spiUnselect(CHIBIOS_SPI_PEREPHERIAL)
#  endif
#else
#   error "Parallel not yet supported under chibios"
#endif




#endif /* STM32F4_CHIBIOS_H_ */
