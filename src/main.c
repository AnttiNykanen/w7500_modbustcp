/*
 * main.c
 *
 *  Created on: Sep 30, 2015
 *      Author: aon
 */

#include "W7500x_gpio.h"

GPIO_InitTypeDef GPIO_InitDef;

int main(int argc, char *argv[])
{
	uint32_t delay = 0;

	SystemInit();

	GPIO_InitDef.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitDef.GPIO_Pin = GPIO_Pin_8;
	GPIO_Init(GPIOC, &GPIO_InitDef);
	PAD_AFConfig(PAD_PC, GPIO_Pin_8, PAD_AF1);

	while(1) {
		GPIO_SetBits(GPIOC, GPIO_Pin_8);
		for (delay = 1250000; delay != 0; delay--);
		GPIO_ResetBits(GPIOC, GPIO_Pin_8);
		for (delay = 1250000; delay != 0; delay--);
	}


	return 0;
}
