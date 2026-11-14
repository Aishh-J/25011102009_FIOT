/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h> 
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART3_UART_Init(void);
/* USER CODE BEGIN PFP */

void send_cmds(char *cmd, uint32_t total_wait_time);
void wifi_connection(void);
void http_server_connection_static(void);
void http_server_connection_dynamic(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */

wifi_connection();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */	
    /* USER CODE BEGIN 3 */
		//http_server_connection_static();
		http_server_connection_dynamic();
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
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

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
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
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void send_cmds(char *cmd, uint32_t total_wait_time) {
	char send[100];
	
	snprintf(send,sizeof(send), "\r\n : %s",cmd);
	
	HAL_UART_Transmit(&huart3,(uint8_t *)send,strlen(send),HAL_MAX_DELAY);
	
	HAL_UART_Transmit(&huart1,(uint8_t *)cmd,strlen(cmd),HAL_MAX_DELAY);
	
	uint8_t rx_byte; 
	uint32_t overall_start = HAL_GetTick();
	uint32_t last_byte_time = HAL_GetTick();
	
	const uint32_t idle_time = 50;
	
	while((HAL_GetTick() - overall_start) < total_wait_time) {
		if(HAL_UART_Receive(&huart1,&rx_byte,1,10) == HAL_OK) {
			
			HAL_UART_Transmit(&huart3,&rx_byte,1,HAL_MAX_DELAY);
			last_byte_time = HAL_GetTick();
		} else if((HAL_GetTick() - last_byte_time) > idle_time) {
			break;
		}
	}
}

void wifi_connection(void) {
	
	HAL_Delay(2000);
	
	//Check and get >OK
	send_cmds("AT\r\n",1000);
	HAL_Delay(1000);
	
	//Set Mode to Station
	send_cmds("AT+CWMODE=1\r\n",1000);
	HAL_Delay(1000);
	
	//Join Access Point, Connect to WIFI Network 
	send_cmds("AT+CWJAP=\"SNUC\", \"snu12345\"\r\n",12000);
	HAL_Delay(1000);
	
	//Get IP Address 
	send_cmds("AT+CIFSR\r\n",3000);
	HAL_Delay(1000);
	
	//Enable multiple connections 
	send_cmds("AT+CIPMUX=1\r\n",1000);
	HAL_Delay(1000);
	
	//Start server to port 80
	send_cmds("AT+CIPSERVER=1,80\r\n",3000);
	HAL_Delay(1000);
	
	//Completed Message 
	char *ready_msg = "\r\nSetup Complete\r\n";
  HAL_UART_Transmit(&huart3, (uint8_t*)ready_msg, strlen(ready_msg), HAL_MAX_DELAY);
}

/*void http_server_connection_static(void) {
    char rx_data[150];
    int rx_index = 0;
    uint32_t start_tick = HAL_GetTick();

    // Listen for data from ESP8266 for up to 100ms
    while ((HAL_GetTick() - start_tick) < 100) {
        uint8_t byte;
        if (HAL_UART_Receive(&huart1, &byte, 1, 10) == HAL_OK) {
            if (rx_index < sizeof(rx_data) - 1) {
                rx_data[rx_index++] = byte;
                rx_data[rx_index] = '\0';
            }
        }
    }

    // Check if an incoming HTTP request (+IPD) arrived from a browser
    char *ipd_ptr = strstr(rx_data, "+IPD,");
    if (ipd_ptr != NULL) {
        int link_id = 0;
        sscanf(ipd_ptr, "+IPD,%d,", &link_id);

        // --- STATIC HTML CONTENT ---
        char html_body[] = "<html><body><h1>Hello from STM32 Static Server!</h1><p>STATIC DATA.</p></body></html>";
        int len = strlen(html_body); // Simply measure the fixed length

        // Construct full HTTP response headers + static body
        char http_response[300];
        int total_len = snprintf(http_response, sizeof(http_response),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %d\r\n\r\n"
            "%s", len, html_body);

        // Send length command to ESP8266
        char send_cmd[30];
        snprintf(send_cmd, sizeof(send_cmd), "AT+CIPSEND=%d,%d\r\n", link_id, total_len);
        HAL_UART_Transmit(&huart1, (uint8_t*)send_cmd, strlen(send_cmd), HAL_MAX_DELAY);
        HAL_Delay(50);

        // Send the HTTP response
        HAL_UART_Transmit(&huart1, (uint8_t*)http_response, total_len, HAL_MAX_DELAY);
        HAL_Delay(50);

        // Close connection
        char close_cmd[30];
        snprintf(close_cmd, sizeof(close_cmd), "AT+CIPCLOSE=%d\r\n", link_id);
        HAL_UART_Transmit(&huart1, (uint8_t*)close_cmd, strlen(close_cmd), HAL_MAX_DELAY);
    }
} */

void http_server_connection_dynamic(void) {
	
	char rx_data[100] = {0};
	char rx_index = 0; 
	uint32_t start_tick = HAL_GetTick(); 
	
	
//Listen for data from ESP8266 for 200ms 
	while((HAL_GetTick() - start_tick) < 200) {
		uint8_t byte; 
		
		if(HAL_UART_Receive(&huart1,&byte,1,10) == HAL_OK) {
			if(rx_index < sizeof(rx_data) - 1) {
				rx_data[rx_index++] = byte; 
				rx_data[rx_index] = '\0';
			}
		}
	}
	
//Find in rx_data where +IPD starts. If found return position else NULL 
	char *ipd_ptr = strstr(rx_data,"+IPD,"); //strstr is String search 
	
	if(ipd_ptr != NULL) {
		//Initial link id starts with 0 
		int link_id = 0;
		sscanf(ipd_ptr, "+IPD,%d,", &link_id);
		
        // You can read a sensor or variable here (e.g., uptime counter or ADC value)
        int uptime_seconds = HAL_GetTick() / 1000;

        char html_body[256];
        int len = snprintf(html_body, sizeof(html_body),
            "<html><head><meta http-equiv=\"refresh\" content=\"2\"></head>"
            "<body>"
            "<h1>STM32 & ESP8266 Web Server</h1>"
            "<p><b>Status:</b> Static & Dynamic Data Working!</p>"
            "<p><b>System Uptime:</b> %d seconds</p>"
            "</body></html>", uptime_seconds);

        // Construct full HTTP response
        char http_response[512];
        int total_len = snprintf(http_response, sizeof(http_response),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %d\r\n\r\n"
            "%s", len, html_body);

        // Send length command to ESP
        char send_cmd[30];
        snprintf(send_cmd, sizeof(send_cmd), "AT+CIPSEND=%d,%d\r\n", link_id, total_len);
        HAL_UART_Transmit(&huart1, (uint8_t*)send_cmd, strlen(send_cmd), HAL_MAX_DELAY);
        HAL_Delay(50);

        // Send the actual HTTP response
        HAL_UART_Transmit(&huart1, (uint8_t*)http_response, total_len, HAL_MAX_DELAY);
        HAL_Delay(50);

        // Close connection
        char close_cmd[30];
        snprintf(close_cmd, sizeof(close_cmd), "AT+CIPCLOSE=%d\r\n", link_id);
        HAL_UART_Transmit(&huart3, (uint8_t*)close_cmd, strlen(close_cmd), HAL_MAX_DELAY);
    }
} 
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
