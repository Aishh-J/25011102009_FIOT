/* Task 9C: Configure the Wemos ESP8266 as a Wi-Fi station, join the AP,
 * query the network status and report it on the PC terminal.
 *
 * Wiring:
 *   PA2 (USART2 TX) -> Wemos RX,  PA3 (USART2 RX) <- Wemos TX   (Wemos)
 *   PA9 (USART1 TX) -> PC USB-TTL RX, PA10 (USART1 RX) <- PC TX (PC)
 */

#include "main.h"
#include <string.h>
#include <stdio.h>

#define WIFI_SSID      "SNUC"
#define WIFI_PASSWORD  "snu12345"

UART_HandleTypeDef huart1;   /* PC    (PA9 TX, PA10 RX) */
UART_HandleTypeDef huart2;   /* Wemos (PA2 TX, PA3 RX)  */

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);

static char resp[300];

/* Send one AT command and collect the reply until OK / ERROR / FAIL or timeout */
static void at_cmd(const char *cmd, uint32_t timeout_ms)
{
    uint16_t n = 0;
    uint8_t c;
    uint32_t t0 = HAL_GetTick();

    memset(resp, 0, sizeof(resp));
    HAL_UART_Transmit(&huart2, (uint8_t *)cmd, strlen(cmd), 1000);

    while ((HAL_GetTick() - t0) < timeout_ms && n < sizeof(resp) - 1)
    {
        if (HAL_UART_Receive(&huart2, &c, 1, 100) == HAL_OK)
        {
            resp[n++] = (char)c;
            resp[n] = '\0';
            if (strstr(resp, "OK\r\n") || strstr(resp, "ERROR\r\n") || strstr(resp, "FAIL\r\n"))
            {
                break;
            }
        }
    }
}

/* Show ">> command" and "<< reply" on the PC, as in the sheet */
static void show(const char *cmd_text)
{
    char line[80];
    const char *r = resp;

    snprintf(line, sizeof(line), ">> %s\r\n", cmd_text);
    HAL_UART_Transmit(&huart1, (uint8_t *)line, strlen(line), 1000);

    /* skip the leading CR/LF the Wemos sends before its reply */
    while (*r == '\r' || *r == '\n') { r++; }

    if (*r == '\0')
    {
        HAL_UART_Transmit(&huart1, (uint8_t *)"<< (no reply)\r\n", 15, 1000);
    }
    else
    {
        snprintf(line, sizeof(line), "<< ");
        HAL_UART_Transmit(&huart1, (uint8_t *)line, strlen(line), 1000);
        HAL_UART_Transmit(&huart1, (uint8_t *)r, strlen(r), 1000);
        if (r[strlen(r) - 1] != '\n')
        {
            HAL_UART_Transmit(&huart1, (uint8_t *)"\r\n", 2, 1000);
        }
    }
}

static void pc_print(const char *s)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)s, strlen(s), 1000);
}

int main(void)
{
    char join_cmd[80];
    char line[120];
    char ip[32] = "NOT FOUND";
    char *p;
    int connected = 0;

    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART1_UART_Init();
    MX_USART2_UART_Init();

    HAL_Delay(2000);   /* let the Wemos boot */

    pc_print("\r\nSTM32-WiFi MODEM TEST\r\n");

    /* 1. AT -> OK */
    at_cmd("AT\r\n", 2000);
    show("AT");

    /* 2. station mode */
    at_cmd("AT+CWMODE=1\r\n", 2000);
    show("AT+CWMODE=1");

    /* 3. join the access point (can take several seconds) */
    snprintf(join_cmd, sizeof(join_cmd), "AT+CWJAP=\"%s\",\"%s\"\r\n", WIFI_SSID, WIFI_PASSWORD);
    at_cmd(join_cmd, 20000);
    snprintf(line, sizeof(line), "AT+CWJAP=\"%s\",\"%s\"", WIFI_SSID, WIFI_PASSWORD);
    show(line);

    /* 4. query network status */
    at_cmd("AT+CWMODE?\r\n", 2000);
    if (strstr(resp, "+CWMODE:1") != NULL)
    {
        pc_print("WIFI MODE    : STATION\r\n");
    }
    else
    {
        pc_print("WIFI MODE    : NOT STATION\r\n");
    }

    at_cmd("AT+CWJAP?\r\n", 2000);
    if (strstr(resp, "+CWJAP:") != NULL)
    {
        connected = 1;
        pc_print("WIFI STATUS  : CONNECTED\r\n");
    }
    else
    {
        pc_print("WIFI STATUS  : DISCONNECTED\r\n");
    }

    at_cmd("AT+CIFSR\r\n", 2000);
    p = strstr(resp, "STAIP,\"");
    if (p != NULL)
    {
        uint8_t i = 0;
        p += strlen("STAIP,\"");
        while (*p != '\"' && *p != '\0' && i < sizeof(ip) - 1)
        {
            ip[i++] = *p++;
        }
        ip[i] = '\0';
    }
    snprintf(line, sizeof(line), "IP ADDRESS    : %s\r\n", ip);
    pc_print(line);

    if (!connected)
    {
        pc_print("Result: not connected (check SSID/password)\r\n");
    }

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
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) { Error_Handler(); }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) { Error_Handler(); }
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
    if (HAL_UART_Init(&huart1) != HAL_OK) { Error_Handler(); }
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
    if (HAL_UART_Init(&huart2) != HAL_OK) { Error_Handler(); }
}

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* USART2 (Wemos): PA2 TX, PA3 RX */
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART1 (PC): PA9 TX, PA10 RX */
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void Error_Handler(void)
{
    __disable_irq();
    while (1) { }
}