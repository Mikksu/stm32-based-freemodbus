/**

  ******************************************************************

  * @file    core_delay.c

  * @author  fire

  * @version V1.0

  * @date    2018-xx-xx

  * @brief   Precise delay using kernel registers

  ******************************************************************

  * @attention

  *

  * Experimental platform: Wildfire STM32H743 development board  

  * Forum: http://www.firebbs.cn

  * TaoBao: https://fire-stm32.taobao.com

  *

  ******************************************************************

  */

  

#include "core_delay.h"   


 

/*

**********************************************************************

*         Timestamp related register definition

**********************************************************************

*/

/*

 In Cortex-M, there is a peripheral called DWT(Data Watchpoint and Trace),

 The peripheral has a 32-bit register called CYCCNT, which is an up counter,

 The number of kernel clocks running is recorded, and the longest recorded time is:

 10.74s=2 32nd power of / 400000000

 (Assuming that the kernel frequency is 400M, the time for one kernel jump is about 1/400M=2.5ns)

 When CYCCNT overflows, it will clear 0 and start counting up again.

 Steps to enable CYCCNT counting:

 1,First enable the DWT peripheral, which is controlled by bit 24 of the other kernel debug register DEMCR, and write 1 enable

 2,Clear 0 before enabling CYCCNT register

 3,Enable the CYCCNT register, which is controlled by DWT_ CTRL (macro defined as DWT on code)_ Bit 0 control of CR), write 1 enable

 */


 

#define  DWT_CR      *(__IO uint32_t *)0xE0001000

#define  DWT_CYCCNT  *(__IO uint32_t *)0xE0001004

#define  DEM_CR      *(__IO uint32_t *)0xE000EDFC


 

#define  DEM_CR_TRCENA                   (1 << 24)

#define  DWT_CR_CYCCNTENA                (1 <<  0)


 

/**

  * @brief  Initialization timestamp

  * @param  nothing

  * @retval nothing

  * @note   This function must be called before using the delay function

  */

HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
    /* Enable DWT peripherals */
    DEM_CR |= (uint32_t)DEM_CR_TRCENA;                

    /* DWT CYCCNT Register count clear 0 */
    DWT_CYCCNT = (uint32_t)0u;

    /* Enable Cortex-M DWT CYCCNT register */
    DWT_CR |= (uint32_t)DWT_CR_CYCCNTENA;

    return HAL_OK;
}

/**

  * @brief  Read current timestamp

  * @param  nothing

  * @retval Current timestamp, i.e. DWT_ Value of cyccnt register

  */

uint32_t CPU_TS_TmrRd(void)
{        
  return ((uint32_t)DWT_CYCCNT);
}

/**

  * @brief  Read current timestamp

  * @param  nothing

  * @retval Current timestamp, i.e. DWT_ Value of cyccnt register

  */

uint32_t HAL_GetTick(void)
{        
  return ((uint32_t)DWT_CYCCNT / GET_CPU_ClkFreq() * 1000);
}


 

/**

  * @brief  The internal counting of CPU is used to realize accurate delay and 32-bit counter

  * @param  us : Delay length in 1 us

  * @retval nothing

  * @note   The CPU must be called before using this function_ TS_ Tmrinit function enables the counter,

            Or enable macro CPU_TS_INIT_IN_DELAY_FUNCTION

            The maximum delay value is 8 seconds, i.e. 8 * 1000 * 1000

  */

void CPU_TS_Tmr_Delay_US(uint32_t us)

{

  uint32_t ticks;

  uint32_t told,tnow,tcnt=0;

  /* Initialize the timestamp register inside the function, */  

#if (CPU_TS_INIT_IN_DELAY_FUNCTION)  

  /* Initialize timestamp and clear */

  HAL_InitTick(5);

#endif

  ticks = us * (GET_CPU_ClkFreq() / 1000000);  /* Number of beats required */      
  tcnt = 0;
  told = (uint32_t)CPU_TS_TmrRd();         /* Counter value when entering */

  while(1)
  {
    tnow = (uint32_t)CPU_TS_TmrRd();  

    if(tnow != told)
    { 
      /* 32 The bit counter is an up counter */
      if(tnow > told)
      {
        tcnt += tnow - told;  
      }
      /* Reload */
      else 
      {
        tcnt += UINT32_MAX - told + tnow; 
      } 

      told = tnow;

      /*If the time exceeds / equals the time to be delayed, exit */

      if(tcnt >= ticks)break;

    }  

  }

}

/*********************************************END OF FILE**********************/
