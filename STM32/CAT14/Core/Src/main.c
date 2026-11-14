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
#include "bme280.h"
#include "ESPLib.h"        
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#ifndef BME280_POWERMODE_NORMAL
#define BME280_POWERMODE_NORMAL  BME280_NORMAL_MODE
#endif

#define BME_I2C_TIMEOUT_MS  100U


#define WIFI_SSID      "SNUC"
#define WIFI_PASS      "snu12345"
#define MQTT_HOST      "broker.emqx.io"
#define MQTT_PORT      "1883"
#define MQTT_CLIENT_ID "stm32bme01"             
#define MQTT_TOPIC     "stm32/bme280/goofy"
#define PUBLISH_PERIOD_MS  1000U


typedef char topic_fits_in_esplib_buffer[((sizeof(MQTT_TOPIC) - 1) + 59 + 4 <= 99) ? 1 : -1];


#define ERR_NONE        0
#define ERR_I2C_READ    1
#define ERR_WRONG_ID    2
#define ERR_INIT        3
#define ERR_SETTINGS    4
#define ERR_SETMODE     5
#define ERR_ESP_NOREPLY 6                       
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
static struct bme280_dev      dev;
static struct bme280_data     comp_data;
static struct bme280_settings settings;
static uint8_t                dev_addr = BME280_I2C_ADDR_PRIM;   

static MQTT_InitTypeDef_t     mqttSetting;       
static char                   json[64];

/* Watch-window variables */
volatile float    temperature, pressure, humidity;
volatile uint8_t  chip_id;
volatile int8_t   rslt;
volatile uint32_t publish_count;
volatile uint8_t  dbg_error;


volatile uint8_t  stage;

//reply recived from esp8266 for the AT cmds
uint8_t           esp_rx[40];
volatile uint16_t esp_rx_len;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */
static int8_t user_i2c_read (uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr);
static int8_t user_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr);
static void   user_delay_us (uint32_t period, void *intf_ptr);
static void   app_error     (uint8_t err, int8_t code);
static void   bme280_setup  (void);
static uint16_t esp_raw_at_test(void);
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
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  //1. Sensor: a wiring fault shows up here, before any Wi-Fi time is spent 
  stage = 0;
  bme280_setup();
  stage = 1;                             

  // 2. Can the STM32 actually HEAR the ESP8266?
  HAL_Delay(1500);                       
  {
    uint8_t tries = 0;
    while (tries < 6)
    {
      esp_rx_len = esp_raw_at_test();
      if ((esp_rx_len > 0) && (strstr((char *)esp_rx, "OK") != NULL)) { break; }
      tries++;
      HAL_Delay(500);
    }
    if (tries >= 6) { app_error(ERR_ESP_NOREPLY, 0); }   
  }
  stage = 2;                             

  //3
  ESP8266_Init(StationMode, huart2);
  stage = 3;                             

  //4
  ESP8266_wifiConnect(WIFI_SSID, WIFI_PASS);
  stage = 4;                             

  //5.stays at 4: no internet or DNS problem.
  ESP8266_portConnect("TCP", MQTT_HOST, MQTT_PORT);
  stage = 5;                             

  //6.mqtt connection
  mqttSetting.keepAlive   = 500;
  mqttSetting.connectFlag = CleanSession;
  mqttSetting.clientID    = MQTT_CLIENT_ID;
  MQTT_connectBroker(&mqttSetting);
  stage = 6;                             

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    stage = 7;                           
    rslt = bme280_get_sensor_data(BME280_ALL, &comp_data, &dev);

    if (rslt == BME280_OK)
    {
        temperature = (float)comp_data.temperature;           
        pressure    = (float)comp_data.pressure / 100.0f;     
        humidity    = (float)comp_data.humidity;              

        
        snprintf(json, sizeof(json),
                 "{\"temperature\":%.2f,\"humidity\":%.2f,\"pressure\":%.2f}",
                 temperature, humidity, pressure);

        MQTT_publishTopic(MQTT_TOPIC, json, QoS0);
        publish_count++;
    }

    HAL_Delay(PUBLISH_PERIOD_MS);
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
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

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
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
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
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* PC13 = onboard LED (error blink). Your CubeMX project did not configure it, so it is set up here. */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  {
    GPIO_InitTypeDef gpio_led = {0};
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);      /* LED off (active low) */
    gpio_led.Pin   = GPIO_PIN_13;
    gpio_led.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio_led.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &gpio_led);
  }
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */


static int8_t user_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    uint8_t addr = *(uint8_t *)intf_ptr;
    return (HAL_I2C_Mem_Read(&hi2c1, (uint16_t)(addr << 1), reg_addr, I2C_MEMADD_SIZE_8BIT,
                             reg_data, (uint16_t)len, BME_I2C_TIMEOUT_MS) == HAL_OK)
           ? BME280_OK : BME280_E_COMM_FAIL;
}

static int8_t user_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    uint8_t addr = *(uint8_t *)intf_ptr;
    return (HAL_I2C_Mem_Write(&hi2c1, (uint16_t)(addr << 1), reg_addr, I2C_MEMADD_SIZE_8BIT,
                              (uint8_t *)reg_data, (uint16_t)len, BME_I2C_TIMEOUT_MS) == HAL_OK)
           ? BME280_OK : BME280_E_COMM_FAIL;
}

static void user_delay_us(uint32_t period, void *intf_ptr)
{
    (void)intf_ptr;
    HAL_Delay((period / 1000U) + 1U);
}

/* Halt + report: reason in dbg_error (Watch window), LED blinks */
static void app_error(uint8_t err, int8_t code)
{
    dbg_error = err;
    rslt      = code;
    while (1)
    {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        HAL_Delay(200);
    }
}

/* Whole BME280 bring-up: chip ID -> init -> settings -> normal mode */
static void bme280_setup(void)
{
    dev.intf     = BME280_I2C_INTF;
    dev.intf_ptr = &dev_addr;
    dev.read     = user_i2c_read;
    dev.write    = user_i2c_write;
    dev.delay_us = user_delay_us;

    rslt = user_i2c_read(BME280_REG_CHIP_ID, (uint8_t *)&chip_id, 1, &dev_addr);   /* 0xD0 */
    if (rslt != BME280_OK)         { app_error(ERR_I2C_READ, rslt); }
    if (chip_id != BME280_CHIP_ID) { app_error(ERR_WRONG_ID, 0);    }              /* 0x60 */

    rslt = bme280_init(&dev);
    if (rslt != BME280_OK) { app_error(ERR_INIT, rslt); }

    settings.osr_h        = BME280_OVERSAMPLING_1X;
    settings.osr_p        = BME280_OVERSAMPLING_1X;
    settings.osr_t        = BME280_OVERSAMPLING_1X;
    settings.filter       = BME280_FILTER_COEFF_OFF;
    settings.standby_time = BME280_STANDBY_TIME_1000_MS;

    rslt = bme280_set_sensor_settings(BME280_SEL_OSR_PRESS | BME280_SEL_OSR_TEMP |
                                      BME280_SEL_OSR_HUM   | BME280_SEL_FILTER   |
                                      BME280_SEL_STANDBY, &settings, &dev);
    if (rslt != BME280_OK) { app_error(ERR_SETTINGS, rslt); }

    rslt = bme280_set_sensor_mode(BME280_POWERMODE_NORMAL, &dev);
    if (rslt != BME280_OK) { app_error(ERR_SETMODE, rslt); }
}

/* Raw handshake with the ESP8266 (no library): sends "AT", stores the reply in esp_rx, returns its length.
   Healthy reply is 10 bytes:  A T \r \n \r \n O K \r \n   */
static uint16_t esp_raw_at_test(void)
{
    uint8_t  b;
    uint16_t n = 0;

    while (HAL_UART_Receive(&huart2, &b, 1, 50) == HAL_OK) { }     /* throw away boot noise / stale bytes */
    memset(esp_rx, 0, sizeof(esp_rx));

    HAL_UART_Transmit(&huart2, (uint8_t *)"AT\r\n", 4, 100);
    while ((n < (sizeof(esp_rx) - 1U)) && (HAL_UART_Receive(&huart2, &b, 1, 300) == HAL_OK))
    {
        esp_rx[n++] = b;
    }
    return n;
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