#include "main.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>

/* UART HANDLES */

UART_HandleTypeDef uart_wemos;
UART_HandleTypeDef uart_pc;

/* RECEIVE BUFFER */

uint8_t rxBuf[64];
uint16_t rxLen = 0;


/* FUNCTION DECLARATIONS */

void GPIO_Init_Custom(void);
void UART_Init_Custom(void);
void Error_Handler(void);

void pc_print(const char *s);


/* PRINT MESSAGE TO PC USING USART2 */

void pc_print(const char *s)
{
    HAL_UART_Transmit(&uart_pc,
                      (uint8_t *)s,
                      strlen(s),
                      1000);
}


/* MAIN FUNCTION */

int main(void)
{
    /* Initialize HAL */

    HAL_Init();


    /* Initialize GPIO */

    GPIO_Init_Custom();


    /* Initialize UART */

    UART_Init_Custom();


    /* START MESSAGE */

    pc_print("\r\n");
    pc_print("********************************\r\n");
    pc_print(" STM32 - Wemos UART Communication\r\n");
    pc_print("********************************\r\n");


    /* INFINITE LOOP */

    while (1)
    {
        /* STEP 1: SEND AT COMMAND */

        const char *cmd = "AT\r\n";

        pc_print("\r\nSTM32 -> Wemos: AT\r\n");

        HAL_UART_Transmit(&uart_wemos,
                          (uint8_t *)cmd,
                          strlen(cmd),
                          1000);


        /* STEP 2: CLEAR BUFFER */

        memset(rxBuf, 0, sizeof(rxBuf));

        rxLen = 0;


        /* STEP 3: RECEIVE RESPONSE */

        if (HAL_UART_Receive(&uart_wemos,
                             &rxBuf[0],
                             1,
                             1000) == HAL_OK)
        {
            rxLen = 1;

            while (rxLen < sizeof(rxBuf) - 1)
            {
                if (HAL_UART_Receive(&uart_wemos,
                                     &rxBuf[rxLen],
                                     1,
                                     100) == HAL_OK)
                {
                    rxLen++;
                }
                else
                {
                    break;
                }
            }


            /* ADD NULL TERMINATOR */

            rxBuf[rxLen] = '\0';


            /* STEP 4: DISPLAY RESPONSE */

            pc_print("Wemos -> STM32: ");

            HAL_UART_Transmit(&uart_pc,
                              rxBuf,
                              rxLen,
                              1000);

            pc_print("\r\n");
        }
        else
        {
            pc_print("No response from Wemos!\r\n");
        }


        /* STEP 5: DELAY */

        HAL_Delay(2000);
    }
}


/* GPIO INITIALIZATION */

void GPIO_Init_Custom(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};


    /* ENABLE GPIO CLOCK */

    __HAL_RCC_GPIOA_CLK_ENABLE();


    /* ENABLE USART CLOCKS */

    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_USART2_CLK_ENABLE();


    /* USART1 TX - PA9 */

    GPIO_InitStruct.Pin = GPIO_PIN_9;

    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;

    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);


    /* USART1 RX - PA10 */

    GPIO_InitStruct.Pin = GPIO_PIN_10;

    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;

    GPIO_InitStruct.Pull = GPIO_NOPULL;

    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);


    /* USART2 TX - PA2 */

    GPIO_InitStruct.Pin = GPIO_PIN_2;

    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;

    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);


    /* USART2 RX - PA3 */

    GPIO_InitStruct.Pin = GPIO_PIN_3;

    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;

    GPIO_InitStruct.Pull = GPIO_NOPULL;

    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}


/* UART INITIALIZATION */

void UART_Init_Custom(void)
{
    /* USART1 - WEMOS */

    uart_wemos.Instance = USART1;

    uart_wemos.Init.BaudRate = 115200;

    uart_wemos.Init.WordLength = UART_WORDLENGTH_8B;

    uart_wemos.Init.StopBits = UART_STOPBITS_1;

    uart_wemos.Init.Parity = UART_PARITY_NONE;

    uart_wemos.Init.Mode = UART_MODE_TX_RX;

    uart_wemos.Init.HwFlowCtl = UART_HWCONTROL_NONE;

    uart_wemos.Init.OverSampling = UART_OVERSAMPLING_16;


    if (HAL_UART_Init(&uart_wemos) != HAL_OK)
    {
        Error_Handler();
    }


    /* USART2 - PC TERMINAL */

    uart_pc.Instance = USART2;

    uart_pc.Init.BaudRate = 115200;

    uart_pc.Init.WordLength = UART_WORDLENGTH_8B;

    uart_pc.Init.StopBits = UART_STOPBITS_1;

    uart_pc.Init.Parity = UART_PARITY_NONE;

    uart_pc.Init.Mode = UART_MODE_TX_RX;

    uart_pc.Init.HwFlowCtl = UART_HWCONTROL_NONE;

    uart_pc.Init.OverSampling = UART_OVERSAMPLING_16;


    if (HAL_UART_Init(&uart_pc) != HAL_OK)
    {
        Error_Handler();
    }
}


/* ERROR HANDLER */

void Error_Handler(void)
{
    __disable_irq();

    while (1)
    {
    }
}