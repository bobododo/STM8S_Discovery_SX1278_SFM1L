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
#include <string.h>
#include "stm8s.h"
#elif defined(STM8L15X_MD)
#include "stm8l15x.h"
#endif
#include "sx1276.h"
#include "task.h"
#include "board.h"

#define INPUT_BUFFER_SIZE 20
#define CMD_BODY_MAX_SIZE 10
#define CMD_OPTIONS_MAX_SIZE 6

#define ISUNSIGNED(a) (a>0 && ~a>0)

typedef enum
{
  PerStart = 0,		
  PerGoOn,
  PerDone
}PER_PHASE;

typedef enum
{
  CMD_INVALID = 0,
  CMD_VALID,
  CMD_SET,
  CMD_QUERY
}CMD_TYPE;

//static uint8_t PER_phase = PerStart;
//static uint8_t RxPacketCout;
static tTaskInstance taskInstance;
static uint8_t total_input_char_number = 0;
static uint8_t input_buffer[INPUT_BUFFER_SIZE];
static s_DeviceParameters devicePara; 

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
  
  EEPROM_Read(DEVICE_PARAMETERS_ADDRESS, (unsigned char *)(&devicePara), sizeof(struct t_DeviceParameters));
  
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

/* command would be like at command */
/* at+xxxx=x,x is for setting */
/* at+xxxx=? is for query */
static void cmd_decoder(uint8_t *p_cmd, uint8_t cmdLen)
{
  if((p_cmd != 0) && (cmdLen <= (CMD_BODY_MAX_SIZE+CMD_OPTIONS_MAX_SIZE+1)))
  {
    uint8_t cmdBody[CMD_BODY_MAX_SIZE];
    uint8_t cmdOptions[CMD_OPTIONS_MAX_SIZE];
    uint8_t i;
    CMD_TYPE cmd_type = CMD_INVALID;
    
    memset(cmdBody, 0, CMD_BODY_MAX_SIZE);
    memset(cmdOptions, 0, CMD_OPTIONS_MAX_SIZE);
    for(i=0;i<cmdLen;i++)
    {
      if(p_cmd[i] == '=')
      {
        /* command body found */
        if(i <= CMD_BODY_MAX_SIZE)
        {
          memcpy(cmdBody, p_cmd, i);
          memcpy(cmdOptions, p_cmd+i+1, CMD_OPTIONS_MAX_SIZE);
          cmdOptions[CMD_OPTIONS_MAX_SIZE-1] = '\0';
          cmd_type = CMD_VALID;
        }
        break;
      }
    }
    
    if(CMD_VALID == cmd_type)
    {
      if((strcmp(cmdBody, "at+on") == 0) || (strcmp(cmdBody, "at+off") == 0))
      {
        uint8_t hostID = 0;
        uint8_t remoteID = 0;
        /* at+on=[hostID],[remoteID] */
        /* at+off=[hostID],[remoteID] */
        /* Only predefined host & remote are allowed */
        if((strlen(cmdOptions) == 5) && (cmdOptions[2] == ','))
        {
          if(cmdOptions[0] >= 0x30 && cmdOptions[0] <= 0x39 && \
             cmdOptions[1] >= 0x30 && cmdOptions[1] <= 0x39 && \
             cmdOptions[3] >= 0x30 && cmdOptions[3] <= 0x39 && \
             cmdOptions[4] >= 0x30 && cmdOptions[4] <= 0x39)
          {
            hostID = (cmdOptions[0]-0x30)*10+cmdOptions[1]-0x30;
            remoteID = (cmdOptions[3]-0x30)*10+cmdOptions[4]-0x30;
            
            if(devicePara.hostID == hostID)
            {
              for(i=0;i<REMOTE_MAX_NUMBER;i++)
              {
                if(devicePara.remoteID[i] == remoteID)
                {
                  tRadioDriver *radio = taskInstance.p_device1; 
                  radio->SetTxPacket((void *)p_cmd, cmdLen);
                  break;
                }
              }
            }
          }
        }
      }
    }
  }
}

void task_exec(tTaskInstance *task)
{
  /* Receive data buffer */
  uint8_t gb_RxData[RF_BUFFER_SIZE];   
  uint16_t packageSize = 0;
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
        radio->GetRxPacket( (void *)gb_RxData, ( uint16_t * )&packageSize );
        
        LoRaRX_Indicate();
          
        Uart_Prints(gb_RxData, packageSize);
        
        //radio->SetTxPacket((void *)gb_RxData, packageSize);
        cmd_decoder(gb_RxData, packageSize);
        
        memset(gb_RxData, 0, RF_BUFFER_SIZE);
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
