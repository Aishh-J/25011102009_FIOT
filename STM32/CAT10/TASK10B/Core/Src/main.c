/* USER CODE BEGIN Header */
/* Task 9C: STM32 - Wemos (ESP8266) Wi-Fi connection, print IP address only */
/* USER CODE END Header */

#include "main.h"
#include <string.h>

UART_HandleTypeDef huart1;   // PC serial monitor (PA9 TX, PA10 RX)
UART_HandleTypeDef huart2;   // Wemos (PA2 TX, PA3 RX)

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);

char rxAT[100];
char rxMode[100];
char rxDhcp[100];
char rxJoin[200];
char rxIP[200];

int main(void)
{
    char title[] = "=== Task 9C: Connect to Wi-Fi ===\r\n";

    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART1_UART_Init();
    MX_USART2_UART_Init();

    HAL_Delay(2000);   // let the Wemos boot

    HAL_UART_Transmit(&huart1, (uint8_t *)title, strlen(title), 1000);

    /* 1. basic response */
    memset(rxAT, 0, sizeof(rxAT));
    HAL_UART_Transmit(&huart2, (uint8_t *)"AT\r\n", 4, 1000);
    HAL_UART_Receive(&huart2, (uint8_t *)rxAT, sizeof(rxAT) - 1, 1000);
    HAL_UART_Transmit(&huart1, (uint8_t *)rxAT, strlen(rxAT), 1000);

    /* 2. station mode */
    memset(rxMode, 0, sizeof(rxMode));
    HAL_UART_Transmit(&huart2, (uint8_t *)"AT+CWMODE=1\r\n", 13, 1000);
    HAL_UART_Receive(&huart2, (uint8_t *)rxMode, sizeof(rxMode) - 1, 2000);
    HAL_UART_Transmit(&huart1, (uint8_t *)rxMode, strlen(rxMode), 1000);

    /* 3. enable DHCP */
    memset(rxDhcp, 0, sizeof(rxDhcp));
    HAL_UART_Transmit(&huart2, (uint8_t *)"AT+CWDHCP=1,1\r\n", 15, 1000);
    HAL_UART_Receive(&huart2, (uint8_t *)rxDhcp, sizeof(rxDhcp) - 1, 2000);
    HAL_UART_Transmit(&huart1, (uint8_t *)rxDhcp, strlen(rxDhcp), 1000);

    /* 4. connect to Wi-Fi */
    memset(rxJoin, 0, sizeof(rxJoin));
    HAL_UART_Transmit(&huart2, (uint8_t *)"AT+CWJAP=\"SNUC\",\"snu12345\"\r\n", 28, 1000);
    HAL_UART_Receive(&huart2, (uint8_t *)rxJoin, sizeof(rxJoin) - 1, 15000);
    HAL_UART_Transmit(&huart1, (uint8_t *)rxJoin, strlen(rxJoin), 1000);

    /* 5. read the IP address and print it to the serial monitor */
    memset(rxIP, 0, sizeof(rxIP));
    HAL_UART_Transmit(&huart2, (uint8_t *)"AT+CIFSR\r\n", 10, 1000);
    HAL_UART_Receive(&huart2, (uint8_t *)rxIP, sizeof(rxIP) - 1, 2000);
    HAL_UART_Transmit(&huart1, (uint8_t *)"\r\n--- IP address ---\r\n", 23, 1000);
    HAL_UART_Transmit(&huart1, (uint8_t *)rxIP, strlen(rxIP), 1000);

    /* done: nothing else to do */
    while (1)
    {
    }
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
}

static void MX_USART1_UART_Init(void)
{
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        Error_Handler();
    }
}

static void MX_USART2_UART_Init(void)
{
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart2) != HAL_OK)
    {
        Error_Handler();
    }
}

static void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
}

void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
}