/*
 * myprintf.h
 *
 *  Created on: Oct 1, 2022
 *      Author: aavila
 */

#ifndef INC_MYPRINTF_H_
#define INC_MYPRINTF_H_
#include "stm32f1xx_hal.h"

void RetargetInit(UART_HandleTypeDef *huart);

int _write(int fd, char* ptr, int len);

void printfRTC(uint8_t *);


#endif /* INC_MYPRINTF_H_ */
