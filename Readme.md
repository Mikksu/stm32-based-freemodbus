[TOC]



## 一、定时器配置

1. 启用定时器
2. 定时器超时时间配置为50us
3. 定时器中断函数中添加如下代码

```c
extern uint16_t downcounter;
if(__HAL_TIM_GET_FLAG(&htim16, TIM_FLAG_UPDATE) != RESET && \
		__HAL_TIM_GET_IT_SOURCE(&htim16, TIM_IT_UPDATE) !=RESET)
{
  __HAL_TIM_CLEAR_IT(&htim16, TIM_IT_UPDATE);
  if (!--downcounter)
    TimerExpiredCallback();
}
```

4. 在`porttimmer.c`中的，将`htimTimeout` 赋值为上述配置好的定时器





## 二、串口配置

1. 串口中断函数中添加如下代码

   ```C
   // the target uart.
   UART_HandleTypeDef *huart = &huart1;
   
   // check if errors occurred.
   uint32_t isrflags   = READ_REG(huart->Instance->SR);
   uint32_t errorflags = 0x00U;
   errorflags = (isrflags & (uint32_t)(USART_SR_PE | USART_SR_FE | USART_SR_ORE | USART_SR_NE));
   if (errorflags == RESET)
   {
     // No error, check the interrupt source.
   
     // 1. char received.
     if((__HAL_UART_GET_FLAG(huart,  UART_FLAG_RXNE) != RESET)
         && (__HAL_UART_GET_IT_SOURCE(huart, UART_IT_RXNE) != RESET))
     {
       pxMBFrameCBByteReceived();
       __HAL_UART_CLEAR_PEFLAG(huart);
     }
   
     // 2. char has been send to DR register.
     else if((__HAL_UART_GET_FLAG(huart, UART_FLAG_TC) !=  RESET)
         && (__HAL_UART_GET_IT_SOURCE(huart, UART_IT_TC) != RESET))
     {
       pxMBFrameCBTransmitterEmpty();
     }
   }
   else
   {
     // clear the errors
     __HAL_UART_CLEAR_PEFLAG(huart);
   
     // set the uart to receive_it mode.
     __HAL_UART_ENABLE_IT(huart, UART_IT_RXNE);
     __HAL_UART_DISABLE_IT(huart, UART_IT_TC);
     __HAL_UART_ENABLE(huart);
   }
   
   return;
   ```

2. 在`portserial.c`文件中，设置RS485 TX/RX 模式切换GPIO

   ```C
   GPIO_TypeDef         *rs485DeGpio         = NULL;
   uint16_t             rs485DePin           = 0;
   ```

3. 如果使用的UART不是**USART1**，则需要在函数`xMBPortSerialInit()`'中增加新的串口支持

   ```C
   BOOL
   xMBPortSerialInit( UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity )
   {
   	/* 
     Do nothing, Initialization is handled by MX_USART3_UART_Init() 
     Fixed port, baudrate, databit and parity  
     */
     // 框架默认只支持USART1，如果Modbus使用其它串口，请在此添加新串口
     if(ucPORT == 1)
       huart = &huart1;
   
     assert(huart != NULL);
   
     HAL_UART_DeInit(huart);
   
     huart->Init.BaudRate = (uint32_t)ulBaudRate;
   
   
     if(eParity ==MB_PAR_ODD)
       huart->Init.Parity = UART_PARITY_ODD;
     else if(eParity ==MB_PAR_EVEN)
         huart->Init.Parity = UART_PARITY_EVEN;
     else
       huart->Init.Parity = UART_PARITY_NONE;
   
     if (HAL_UART_Init(huart) != HAL_OK)
      {
        Error_Handler();
      }
   
   	return TRUE;
   }
   ```

   

## 三、启动Modbus任务

1. 定义任务

   在`freertos.c`头部定义任务句柄和任务函数：

   ```C
   osThreadId modbusPollingTaskHandle;
   void StartModbusPollingTask(void const * argument);
   ```
   在`freertos.c`中添加任务函数：

   ```C
   void StartModbusPollingTask(void const * argument)
   {
   	eMBInit(MB_RTU, 0X01, 1, 115200, MB_PAR_NONE);
   	eMBEnable();
   	for(;;)
   	{
   		eMBPoll();
   	}
   }
   ```
   
2. 启动任务
   在`MX_FREERTOS_Init(void)`函数中添加下列代码以启动任务：
   
   ```C
   osThreadDef(modbusPollingTask, StartModbusPollingTask,  osPriorityNormal, 0, 256);
   modbusPollingTaskHandle =  osThreadCreate(osThread(modbusPollingTask), NULL);
   ```
   > 如果未使用RTOS，则直接在Main函数循环中调用`eMBPoll()`。