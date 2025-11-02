
/*
 * myprintf.c
 *
 *  Created on: Oct 1, 2022
 *      Author: aavila
 */


#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include "main.h"
#include "myprintf.h"

/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
//PUTCHAR_PROTOTYPE {
//  /* write a character to the USART3 and Loop until the end of transmission*/
//  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
//  return ch;
//}

UART_HandleTypeDef* gHuart;

void RetargetInit(UART_HandleTypeDef *huart2) {
  gHuart = huart2;

  /* Disable I/O buffering for STDOUT stream, so that
   * chars are sent out as soon as they are printed. */
  setvbuf(stdout, NULL, _IONBF, 0);
}

int _write(int fd, char* ptr, int len) {

  HAL_UART_Transmit(gHuart, (uint8_t *) ptr, len, HAL_MAX_DELAY);
  return len;
}


//PUTCHAR_PROTOTYPE {
//  /* write a character to the USART3 and Loop until the end of transmission*/
//  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
//  return ch;
//}

void printfRTC(uint8_t* buff){
	printf("Day:%x, Month:%x, Year:20%x, Hour:%x, Minutes:%x, Seconds:%x\n\r",buff[4],buff[5],buff[6],buff[2],buff[1],buff[0]);
}


