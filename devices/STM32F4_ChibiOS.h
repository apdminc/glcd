#ifndef STM32F4_CHIBIOS_H_
#define STM32F4_CHIBIOS_H_

#include "ch.h"
#include "hal.h"

#if !defined(GLCD_USE_PARALLEL)
    #define GLCD_SELECT()     spiSelect(CHIBIOS_SPI_PEREPHERIAL)
    #define GLCD_DESELECT()   spiUnselect(CHIBIOS_SPI_PEREPHERIAL)
#else
#   error "Parallel not yet supported under chibios"
#endif




#endif /* STM32F4_CHIBIOS_H_ */
