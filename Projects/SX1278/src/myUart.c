/******************************************************************************
 * Project        : STM38S003+SX1278
 * File           : myUart.c
 * Copyright      : 2014 Yosun Singapore Pte Ltd
 ******************************************************************************
  Change History:

    Version 1.0 - Aug 2014
    > Initial revision

******************************************************************************/
#include <iostm8s003k3.h>

void uart1_init(void)
{
  UART1_CR2 = 0x24;
  UART1_SR = 0;
  UART1_CR1 = 0;
  UART1_CR3 = 0;   
  // baud rate 115200
  //UART1_BRR1 = 0x08; 
  //UART1_BRR2 = 0x0B;
  // baud rate 9600
  UART1_BRR1 = 0x68; 
  UART1_BRR2 = 0x03;
}

void send_char_com(unsigned char UtxData)
{  
	UART1_CR2_TEN=1;
 	while(!UART1_SR_TXE);
  		UART1_DR = UtxData;         
   	while(!UART1_SR_TC);//TC==0 
		UART1_CR2_TEN=0;
}

unsigned char get_char_com(void)
{
  return UART1_DR;
}

void Uart_Prints(unsigned char *pd)
{
  while((*pd)!='\0')
  {
    send_char_com(*pd);
    pd++;
  }
}

void Uart_Prints2(unsigned char *pd, int len)
{
  int i;
  
  for(i=0;i<len;i++)
  {
    send_char_com(pd[i]);
  }
}

void HexToAscii_AndUartSent(unsigned char Hex)
{
  unsigned char Ascii[3];
  
  Ascii[0]=(Hex/100)+0x30;
  Ascii[1]=((Hex%100)/10)+0x30;
  Ascii[2]=(Hex%10)+0x30;
  
  send_char_com(Ascii[0]);
  send_char_com(Ascii[1]);
  send_char_com(Ascii[2]);
}