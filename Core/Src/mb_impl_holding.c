#include "port.h"
#include "mb.h"
#include "mbutils.h"

#define REG_HOLDING_START 0
#define REG_HOLDING_NREGS 100
static USHORT usRegHoldingStart = REG_HOLDING_START;
USHORT usRegHoldingBuf[REG_HOLDING_NREGS] = {0, };

#ifdef MB_OS_USED
#include "cmsis_os.h"
#include "mb_os_def.h"

osMessageQDef(msgQueueHolding, 5, uint32_t);
osMessageQId msgQueueHoldingHandle;

osPoolDef(poolHoldingMsg, 5, MB_MSG_TypeDef);
osPoolId poolHoldingMsgHandle;

osThreadId regHoldingTaskHandle;
osThreadId dutCommTaskhandle;

void StartTaskRegHolding(void const * argument)
{
  // create the message queue.
  msgQueueHoldingHandle = osMessageCreate(osMessageQ(msgQueueHolding), NULL);

  // create the message pool.
  poolHoldingMsgHandle = osPoolCreate(osPool(poolHoldingMsg));

  for(;;)
  {
    osEvent evt = osMessageGet(msgQueueHoldingHandle, osWaitForever);
    if(evt.status == osEventMessage)
    {
        // get the pointer of the modbus message struct.
        MB_MSG_TypeDef* msg = evt.value.p;
        if(msg != NULL)
        {
          // some of the input regs are changed.
          int nRegs = msg->NRegs;
          int regIndex = msg->RegIndex;
         // uint16_t regVal;

          while(nRegs > 0)
          {
            switch(regIndex)
            {

							/*
              case REG_HOLDING_POS_DAC_OUTPUT:
                //Top_TecSetDacVolt(env->TECConf.DacOutputMv);
                break;

              case REG_HOLDING_POS_EXECUTE:  // Env Operation
                regVal = usRegHoldingBuf[regIndex];
                usRegHoldingBuf[regIndex] = 0x0;

                if (regVal == 17747) // Save Env to the flash
                {
                  Top_SaveEnvToFlash();
                  Top_SetErrorCode(ERR_NO);
                  usRegHoldingBuf[REG_HOLDING_POS_EXECUTE] = 0;
                }
                else if (regVal == 17740) // Reload Env from the flash
                {
                  Top_LoadEnvFromFlash();
                  Top_SetErrorCode(ERR_NO);
                  usRegHoldingBuf[REG_HOLDING_POS_EXECUTE] = 0;
                }
                else if (regVal == 18770) // Read data from device via I2C
                {
                  osSignalSet(dutCommTaskhandle, 0x1);
                }
                else if (regVal == 18775) // Write data to device via I2C
                {
                  osSignalSet(dutCommTaskhandle, 0x2);
                }
								*/
            }

            nRegs--;
            regIndex++;
          }

          osPoolFree(poolHoldingMsgHandle, (void*)msg);
        }
    }
  }
}


void StartTaskDutComm(void const * argument)
{
	/*
	uint8_t i2cBuf[MAX_SIZE_DUT_I2C_BUF];
  for(;;)
  {
    osEvent evt = osSignalWait(0x1 | 0x2, osWaitForever);
    if(evt.status == osEventSignal)
    {
      // limit the length per read/write to 10 bytes.
      if(dutI2c->RegLength > MAX_SIZE_DUT_I2C_BUF)
        dutI2c->RegLength = MAX_SIZE_DUT_I2C_BUF;

      if(evt.value.signals == 0x1) // I2C read
      {
        int ret = I2C_Master_MemRead((uint8_t)dutI2c->SlaveAddress, (uint8_t)dutI2c->RegStart, (uint8_t)dutI2c->RegLength, i2cBuf);
        if(ret == 0)
        {
          for(int i = 0; i < dutI2c->RegLength; i++)
          {
            dutI2c->Data[i] = (uint16_t)i2cBuf[i];
          }

          Top_SetErrorCode(ERR_NO);
        }
        else if(ret == -1)
        {
          Top_SetErrorCode(ERR_DUT_I2C_NO_ACK);
        }
        else if(ret == -2)
        {
          Top_SetErrorCode(ERR_DUT_I2C_BUS_BUSY);
        }
        else
        {
          Top_SetErrorCode(ERR_UNDEFINED);
        }
      }
      else if(evt.value.signals == 0x2) // I2C write
      {
        // convert the data written to the DUT to the byte array.
        for(int i = 0; i < MAX_SIZE_DUT_I2C_BUF; i++)
        {
          i2cBuf[i] = (uint8_t)dutI2c->Data[i];
        }
        int ret = I2C_Master_MemWrite((uint8_t)dutI2c->SlaveAddress, (uint8_t)dutI2c->RegStart, (uint8_t)dutI2c->RegLength, i2cBuf);
        if(ret == 0)
        {
          Top_SetErrorCode(ERR_NO);
        }
        else if(ret == -1)
        {
          Top_SetErrorCode(ERR_DUT_I2C_NO_ACK);
        }
        else
        {
          Top_SetErrorCode(ERR_UNDEFINED);
        }
      }

			Top_AddI2CCommCounter();


			// reset the 'Operation Code'.
      usRegHoldingBuf[REG_HOLDING_POS_EXECUTE] = 0;

			
    }
  }
	*/
}

/*
 * Create the task to process the coil registers.
 */
void CreateMbHoldingProcTask(void)
{
  osThreadDef(regHoldingTask, StartTaskRegHolding, osPriorityNormal, 0, 256);
  regHoldingTaskHandle = osThreadCreate(osThread(regHoldingTask), NULL);

  osThreadDef(dutCommTask, StartTaskDutComm, osPriorityNormal, 0, 256);
  dutCommTaskhandle = osThreadCreate(osThread(dutCommTask), NULL);
}

#endif


/**
 * Modbus slave holding register callback function.
 *
 * @param pucRegBuffer holding register buffer
 * @param usAddress holding register address
 * @param usNRegs holding register number
 * @param eMode read or write
 *
 * @return result
 */
eMBErrorCode eMBRegHoldingCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode)
{
    eMBErrorCode eStatus = MB_ENOERR;
    USHORT iRegIndex;

#ifdef MB_OS_USED
    MB_MSG_TypeDef* msg;
#endif


    /* it already plus one in modbus function method. */
    usAddress--;

    if ((usAddress >= usRegHoldingStart) && (usAddress + usNRegs <= usRegHoldingStart + REG_HOLDING_NREGS))
    {
        iRegIndex = usAddress - usRegHoldingStart;
        switch (eMode)
        {
        /* read current register values from the protocol stack. */
        case MB_REG_READ:
            while (usNRegs > 0)
            {
                *pucRegBuffer++ = (UCHAR)(usRegHoldingBuf[iRegIndex] >> 8);
                *pucRegBuffer++ = (UCHAR)(usRegHoldingBuf[iRegIndex] & 0xFF);
                iRegIndex++;
                usNRegs--;
            }
            break;

        /* write current register values with new values from the protocol stack. */
        case MB_REG_WRITE:

#ifdef MB_OS_USED
            // send a message to tell the task there are some registers are set.

            msg = osPoolCAlloc(poolHoldingMsgHandle);
            if(msg != NULL)
            {
               msg->RegIndex = iRegIndex;
               msg->NRegs = usNRegs;
               msg->RegBitIndex = 0;
            }
#endif
            while (usNRegs > 0)
            {
                usRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
                usRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
                iRegIndex++;
                usNRegs--;
            }

#ifdef MB_OS_USED
            if(msg != NULL)
              osMessagePut(msgQueueHoldingHandle, (uint32_t)msg, 100);

#endif
            break;
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}

