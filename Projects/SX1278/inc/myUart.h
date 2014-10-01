/******************************************************************************
 * Project        : STM38S003+SX1278
 * File           : myUart.h
 * Copyright      : 2014 Yosun Singapore Pte Ltd
 ******************************************************************************
  Change History:

    Version 1.0 - Aug 2014
    > Initial revision

******************************************************************************/
#ifndef _MYUART_H_
#define _MYUART_H_

void uart1_init(void);
void send_char_com(unsigned char UtxData);
unsigned char get_char_com(void);
void Uart_Prints(unsigned char *pd);
void Uart_Prints2(unsigned char *pd, int len);
void HexToAscii_AndUartSent(unsigned char Hex);
#endif
