/******************************************************************************
 * Project        : STM8S_Discovery_SX1278_SFM1L
 * File           : task.c
 * Copyright      : 2014 Yosun Singapore Pte Ltd
 ******************************************************************************
  Change History:

    Version 1.0 - Sep 2014
    > Initial revision

******************************************************************************/
#if defined(STM8S003)
#include "stm8s.h"
#elif defined(STM8L15X_MD)
#include "stm8l15x.h"
#endif
#include "sx1276.h"
#include "task.h"
#include "board.h"

#define INPUT_BUFFER_SIZE 50

typedef enum
{
  PerStart = 0,		
  PerGoOn,
  PerDone//
}PER_PHASE;

static uint8_t gb_RxData[256];                                         //Receive data buffer
static uint16_t packageSize = 0;
static uint8_t PER_phase = PerStart;
static uint8_t RxPacketCout;
static tTaskInstance taskInstance;
static uint8_t total_input_char_number = 0;
static uint8_t input_buffer[INPUT_BUFFER_SIZE];

tTaskInstance* task_init(void)
{
  uint8_t i;
  
  for(i=0;i<INPUT_BUFFER_SIZE;i++)
  {
    input_buffer[i] = 0;
  }
  
  taskInstance.p_device1 = 0;
  taskInstance.p_data = input_buffer;
  taskInstance.p_dataLen = &total_input_char_number;
  
  return (&taskInstance);
}

static void discard_input_buffer(void)
{
  uint8_t i;
  
  for(i=0;i<INPUT_BUFFER_SIZE;i++)
  {
    input_buffer[i] = 0;
  }
  
  total_input_char_number = 0;
}

void task_exec(tTaskInstance *task)
{
  tRadioDriver *radio;
  
  if(task == 0)
  {
    return;
  }
  
  radio = (tRadioDriver *)task->p_device1;
  if(radio == 0)
  {
    return;
  }
   
  disableInterrupts();
  /* check if the last char is Carriage Return(CR) */
  /* If yes, then decode the message */
  /* If no, then discard the buffer if max buffer size reach */
  /* Otherwise, wait for CR */
  if((total_input_char_number > 0) && (input_buffer[total_input_char_number-1] == '\r'))
  {
    //Uart_Prints2(input_buffer, total_input_char_number);
    
    /* No transmit UART RX data through RF */  
    //radio->SetTxPacket(input_buffer, total_input_char_number);
    
    discard_input_buffer();
  }
  else if(total_input_char_number == INPUT_BUFFER_SIZE)
  {
      discard_input_buffer();
  }
  enableInterrupts();
  
  switch( radio->Process( ) )
  {
    case RF_RX_TIMEOUT:
        break;
    case RF_RX_DONE:
        radio->GetRxPacket( gb_RxData, ( uint16_t * )&packageSize );
        
        LoRaRX_Indicate();
          
        //PER_Proc();
        Uart_Prints(gb_RxData, packageSize);
        
        radio->SetTxPacket(gb_RxData, packageSize);
        break;
    case RF_TX_DONE:
        radio->StartRx( );
        break;
    default:
        //send_char_com('K');
        break;
  }
}

void get_input(void)
{
  /* discard the receiving char if buffer data is not handled by task handler */
  if(total_input_char_number < INPUT_BUFFER_SIZE)
  {
#if defined(STM8S003)
    input_buffer[total_input_char_number++] = UART1->DR;
#elif defined(STM8L15X_MD)
    input_buffer[total_input_char_number++] = USART1->DR;
#endif
  }
}
