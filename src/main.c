/*
 * main.c
 *
 *  Created on: Sep 30, 2015
 *      Author: aon
 */

#include <stdint.h>
#include <stdio.h>

#include "W7500x_crg.h"
#include "W7500x_gpio.h"
#include "W7500x_uart.h"
#include "W7500x_wztoe.h"
#include "W7500x_miim.h"

#include "mb_common.h"
#include "mb_register.h"
#include "mbtcp_server.h"

static void UART_Setup(void);
void delay(__IO uint32_t milliseconds);
void TimingDelay_Decrement(void);

static __IO uint32_t TimingDelay;
uint8_t mbtcp_socklist[MBTCP_NSOCK] = {0, 1, 2, 3};

#define HOLDING_REGISTERS_SIZE 16
uint16_t holding_registers[HOLDING_REGISTERS_SIZE];

#define COIL_REGISTERS_SIZE 16
uint8_t coil_registers[COIL_REGISTERS_SIZE];

#define INPUT_REGISTERS_SIZE 16
uint16_t input_registers[INPUT_REGISTERS_SIZE];

#define DISCRETE_REGISTERS_SIZE 16
uint8_t discrete_registers[DISCRETE_REGISTERS_SIZE];

int main(int argc, char *argv[])
{
    GPIO_InitTypeDef GPIO_InitStruct;
    uint32_t system_clock;
    uint8_t i;

    uint8_t hw_addr[] = {0x00, 0x08, 0xDC, 0x12, 0x34, 0x56};
    uint8_t ip_addr[] = {10, 0, 0, 25};
    uint8_t gateway[] = {10, 0, 0, 1};
    uint8_t netmask[] = {0xFF, 0xFF, 0xFF, 0x00};

    SystemInit();

    system_clock = GetSystemClock();

    SysTick_Config(system_clock / 1000);
    setTIC100US(system_clock  /10000);

    UART_Setup();

    setbuf(stdout, NULL);

    // IC+ 101AG pull-ups + driving strength
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13;
    GPIO_InitStruct.GPIO_Pad = GPIO_IE | GPIO_DS | GPIO_SUMMIT;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    setSHAR(hw_addr);
    setSIPR(ip_addr);
    setGAR(gateway);
    setSUBR(netmask);

    mdio_init(GPIOB, MDC, MDIO);

    /* Initialize holding registers */
    for (i = 0; i < HOLDING_REGISTERS_SIZE; i++) {
        holding_registers[i] = 0xAA50 + i;
        printf("%x\r\n", holding_registers[i]);
    }
    mb_register_holding_init(holding_registers, HOLDING_REGISTERS_SIZE);

    /* Initialize coil registers */
    mb_register_coil_init(coil_registers, COIL_REGISTERS_SIZE);

    /* Initialize discrete registers */
    for (i = 0 ; i < DISCRETE_REGISTERS_SIZE; i++) {
        discrete_registers[i] = 0xD0 + i;
    }
    mb_register_discrete_init(discrete_registers, DISCRETE_REGISTERS_SIZE);

    /* Initialize input registers */
    for (i = 0; i < INPUT_REGISTERS_SIZE; i++) {
        input_registers[i] = 0xCC30 + i + 1;
    }
    mb_register_input_init(input_registers, INPUT_REGISTERS_SIZE);

    /* Initialize MB TCP server */
    mbtcp_server_init(mbtcp_socklist, MBTCP_NSOCK);

    while(1) {
        for(i = 0; i < MBTCP_NSOCK; i++) {
            mbtcp_server_run(i);
        }
    }

    return 0;
}

static void UART_Setup(void)
{
    UART_InitTypeDef UART_InitStruct;
    UART_StructInit(&UART_InitStruct);
    UART_Init(UART0, &UART_InitStruct);
}

/**
 * @brief  Inserts a delay time.
 * @param  nTime: specifies the delay time length, in milliseconds.
 * @retval None
 */
void delay(__IO uint32_t milliseconds)
{
    TimingDelay = milliseconds;

    while(TimingDelay != 0);
}

/**
 * @brief  Decrements the TimingDelay variable.
 * @param  None
 * @retval None
 */
void TimingDelay_Decrement(void)
{
    if (TimingDelay != 0x00)
    {
        TimingDelay--;
    }
}
