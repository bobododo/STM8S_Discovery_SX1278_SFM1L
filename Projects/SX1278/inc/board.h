/******************************************************************************
 * Project        : STM8S_Discovery_SX1278_SFM1L
 * File           : board.h
 * Copyright      : 2014 Yosun Singapore Pte Ltd
 ******************************************************************************
  Change History:

    Version 1.0 - Sep 2014
    > Initial revision

******************************************************************************/
#ifndef _BOARD_H_
#define _BOARD_H_

void board_init(void);
void LoRaRX_Indicate(void);
void Uart_Prints(unsigned char *p_data, int length);
#endif /* _BOARD_H_ */
