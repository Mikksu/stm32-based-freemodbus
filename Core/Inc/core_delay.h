#ifndef __CORE_DELAY_H
#define __CORE_DELAY_H

#include "stm32f4xx.h"

/* Get kernel clock frequency */
#define GET_CPU_ClkFreq()       HAL_RCC_GetSysClockFreq()
//#define SysClockFreq            (96e6)
/* For convenience, the CPU is called inside the delay function_ TS_ Tmrinit function initializes the timestamp register,
  In this way, the function will be initialized every time it is called.
  Set the macro value to 0, and then call the CPU when the main function is just running_ TS_ Tmrinit avoids initialization every time */

#define CPU_TS_INIT_IN_DELAY_FUNCTION   0


/*******************************************************************************
* Function declaration
******************************************************************************/
uint32_t CPU_TS_TmrRd(void);
HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority);

//The CPU must be called before using the following functions_ TS_ Tmrinit function enables the counter or macro CPU_TS_INIT_IN_DELAY_FUNCTION
//The maximum delay value is 8 seconds
void CPU_TS_Tmr_Delay_US(uint32_t us);
#define HAL_Delay(ms)               CPU_TS_Tmr_Delay_US(ms*1000)
#define CPU_TS_Tmr_Delay_S(s)       CPU_TS_Tmr_Delay_MS(s*1000)


#endif /* __CORE_DELAY_H */
