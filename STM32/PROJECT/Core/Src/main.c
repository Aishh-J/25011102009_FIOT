/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : BME280 -> STM32 -> Wemos (AT) -> MQTT weather station
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "bme280.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define WIFI_SSID        "SNUC"
#define WIFI_PASS        "snu12345"
#define MQTT_HOST        "broker.emqx.io"
#define MQTT_PORT        1883
#define MQTT_TOPIC       "snuc/fpiot/yellow/bme280"
#define MQTT_CLIENT_ID   "bluepill-weather"
#define PUBLISH_PERIOD_MS 8000
#define RX_BUF_SIZE      512
#define STATUS_PORT      GPIOC
#define STATUS_PIN       GPIO_PIN_13      /* on-board LED, active low */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
struct bme280_dev dev;
struct bme280_data comp;
struct bme280_settings settings;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

static char rx_buf[RX_BUF_SIZE];
static uint16_t rx_len = 0;
static uint8_t wifi_ready = 0;
static uint8_t mqtt_ready = 0;
static uint8_t mqtt_fails = 0;

/* ---------- Debug output on USART1 (PA9), read by the Arduino bridge ---------- */
static void dbg(const char *fmt, ...)
{
  char buf[384];
  va_list ap;
  va_start(ap, fmt);
  int n = vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);
  if (n > 0)
  {
    if (n > (int)sizeof(buf) - 1) n = sizeof(buf) - 1;
    HAL_UART_Transmit(&huart1, (uint8_t *)buf, (uint16_t)n, 200);
  }
  HAL_UART_Transmit(&huart1, (uint8_t *)"\r\n", 2, 100);
}

/* ---------- Wemos (ESP8266 AT firmware) on USART2 (PA2 TX, PA3 RX) ---------- */
static void esp_send_raw(const uint8_t *d, uint16_t len)
{
  HAL_UART_Transmit(&huart2, (uint8_t *)d, len, 1000);
}

static void esp_send_line(const char *s)
{
  esp_send_raw((const uint8_t *)s, (uint16_t)strlen(s));
  esp_send_raw((const uint8_t *)"\r\n", 2);
}

static void rx_clear(void)
{
  rx_len = 0;
  rx_buf[0] = 0;
}

static void rx_append(uint8_t c)
{
  if (rx_len < RX_BUF_SIZE - 1)
  {
    rx_buf[rx_len++] = (char)c;
    rx_buf[rx_len] = 0;
  }
}

/* Reads one byte. Clears overrun errors so reception keeps going. */
static int rx_getbyte(uint8_t *c)
{
  if (HAL_UART_Receive(&huart2, c, 1, 2) == HAL_OK) return 1;
  __HAL_UART_CLEAR_OREFLAG(&huart2);
  return 0;
}

/* Waits until tok1 or tok2 appears. Returns 1 (tok1), 2 (tok2) or 0 (timeout). */
static int esp_wait(const char *tok1, const char *tok2, uint32_t timeout_ms)
{
  uint32_t start = HAL_GetTick();
  uint8_t c;
  while ((HAL_GetTick() - start) < timeout_ms)
  {
    if (rx_getbyte(&c))
    {
      rx_append(c);
      if (strstr(rx_buf, tok1)) return 1;
      if (tok2 && strstr(rx_buf, tok2)) return 2;
    }
  }
  return 0;
}

/* Collects everything the ESP sends for timeout_ms (binary safe). */
static void esp_collect(uint32_t timeout_ms)
{
  uint32_t start = HAL_GetTick();
  uint8_t c;
  while ((HAL_GetTick() - start) < timeout_ms)
  {
    if (rx_getbyte(&c)) rx_append(c);
  }
}

/* Sends an AT command and returns 1 if it ends with OK. */
static int esp_cmd(const char *cmd, uint32_t timeout_ms)
{
  rx_clear();
  esp_send_line(cmd);
  int r = esp_wait("OK", "ERROR", timeout_ms);
  dbg("> %s", cmd);
  dbg("< %s", rx_buf);
  return r == 1;
}

/* Sends raw bytes with AT+CIPSEND and waits for SEND OK. */
static int esp_send_bytes(const uint8_t *data, uint16_t len)
{
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "AT+CIPSEND=%u", (unsigned)len);
  rx_clear();
  esp_send_line(cmd);
  if (esp_wait(">", "ERROR", 3000) != 1)
  {
    dbg("CIPSEND prompt missing");
    return 0;
  }
  esp_send_raw(data, len);
  if (esp_wait("SEND OK", "ERROR", 3000) != 1)
  {
    dbg("SEND OK not received");
    return 0;
  }
  return 1;
}

/* ---------- Wi-Fi ---------- */
static int wifi_join(void)
{
  char cmd[128];
  int ok = 0;

  for (int i = 0; i < 3 && !ok; i++) ok = esp_cmd("AT", 1000);
  if (!ok)
  {
    dbg("No reply from Wemos: check TX/RX wires, 5V and AT firmware");
    return 0;
  }
  esp_cmd("ATE0", 1000);                 /* turn off echo */
  esp_cmd("AT+CWMODE=1", 2000);          /* station mode */

  snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"", WIFI_SSID, WIFI_PASS);
  if (!esp_cmd(cmd, 20000))
  {
    dbg("Wi-Fi join FAILED");
    return 0;
  }
  esp_cmd("AT+CIPMUX=0", 2000);          /* single connection */
  dbg("Wi-Fi OK");
  return 1;
}

/* ---------- MQTT packet building (MQTT 3.1.1, QoS 0) ---------- */
static uint16_t mqtt_encode_len(uint8_t *p, uint32_t len)
{
  uint16_t n = 0;
  do
  {
    uint8_t d = len % 128;
    len /= 128;
    if (len > 0) d |= 0x80;
    p[n++] = d;
  } while (len > 0);
  return n;
}

static uint16_t mqtt_build_connect(uint8_t *out)
{
  uint8_t body[64];
  uint16_t b = 0;
  uint16_t cl = (uint16_t)strlen(MQTT_CLIENT_ID);

  body[b++] = 0x00; body[b++] = 4;             /* protocol name length */
  memcpy(&body[b], "MQTT", 4); b += 4;
  body[b++] = 4;                               /* protocol level 3.1.1 */
  body[b++] = 0x02;                            /* clean session */
  body[b++] = 0x00; body[b++] = 60;            /* keep alive 60 s */
  body[b++] = (uint8_t)(cl >> 8); body[b++] = (uint8_t)(cl & 0xFF);
  memcpy(&body[b], MQTT_CLIENT_ID, cl); b += cl;

  out[0] = 0x10;                               /* CONNECT */
  uint16_t h = mqtt_encode_len(&out[1], b);
  memcpy(&out[1 + h], body, b);
  return (uint16_t)(1 + h + b);
}

static uint16_t mqtt_build_publish(uint8_t *out, const char *topic, const char *payload)
{
  uint8_t body[256];
  uint16_t b = 0;
  uint16_t tl = (uint16_t)strlen(topic);
  uint16_t pl = (uint16_t)strlen(payload);

  body[b++] = (uint8_t)(tl >> 8); body[b++] = (uint8_t)(tl & 0xFF);
  memcpy(&body[b], topic, tl); b += tl;
  memcpy(&body[b], payload, pl); b += pl;

  out[0] = 0x30;                               /* PUBLISH, QoS 0 */
  uint16_t h = mqtt_encode_len(&out[1], b);
  memcpy(&out[1 + h], body, b);
  return (uint16_t)(1 + h + b);
}

static int bytes_contain(const uint8_t *buf, uint16_t len, const uint8_t *pat, uint16_t plen)
{
  for (uint16_t i = 0; i + plen <= len; i++)
  {
    if (memcmp(&buf[i], pat, plen) == 0) return 1;
  }
  return 0;
}

/* ---------- MQTT connect and publish ---------- */
static int mqtt_connect(void)
{
  char cmd[128];
  uint8_t pkt[64];
  const uint8_t connack[4] = {0x20, 0x02, 0x00, 0x00};

  esp_cmd("AT+CIPCLOSE", 1000);          /* close any old socket, error is OK */

  snprintf(cmd, sizeof(cmd), "AT+CIPSTART=\"TCP\",\"%s\",%d", MQTT_HOST, MQTT_PORT);
  if (!esp_cmd(cmd, 10000))
  {
    dbg("MQTT connect FAILED: TCP");
    return 0;
  }

  uint16_t n = mqtt_build_connect(pkt);
  if (!esp_send_bytes(pkt, n))
  {
    dbg("MQTT connect FAILED: CONNECT not sent");
    return 0;
  }

  esp_collect(1500);                     /* wait for CONNACK */
  if (!bytes_contain((uint8_t *)rx_buf, rx_len, connack, 4))
  {
    dbg("MQTT connect FAILED: no CONNACK");
    return 0;
  }
  dbg("MQTT connected to %s", MQTT_HOST);
  return 1;
}

static int mqtt_publish(const char *payload)
{
  uint8_t pkt[300];
  uint16_t n = mqtt_build_publish(pkt, MQTT_TOPIC, payload);
  if (!esp_send_bytes(pkt, n))
  {
    dbg("publish FAILED");
    return 0;
  }
  dbg(">> published: %s", payload);
  return 1;
}

/* ---------- BME280 (Bosch driver, I2C on PB6/PB7) ---------- */
static BME280_INTF_RET_TYPE bme_read(uint8_t reg, uint8_t *data, uint32_t len, void *intf)
{
  (void)intf;
  return (HAL_I2C_Mem_Read(&hi2c1, (0x76 << 1), reg, I2C_MEMADD_SIZE_8BIT,
                           data, len, 100) == HAL_OK) ? BME280_INTF_RET_SUCCESS : BME280_E_COMM_FAIL;
}

static BME280_INTF_RET_TYPE bme_write(uint8_t reg, const uint8_t *data, uint32_t len, void *intf)
{
  (void)intf;
  return (HAL_I2C_Mem_Write(&hi2c1, (0x76 << 1), reg, I2C_MEMADD_SIZE_8BIT,
                            (uint8_t *)data, len, 100) == HAL_OK) ? BME280_INTF_RET_SUCCESS : BME280_E_COMM_FAIL;
}

static void bme_delay(uint32_t period_us, void *intf)
{
  (void)intf;
  HAL_Delay(period_us / 1000 + 1);
}

static int8_t bme_init_sensor(void)
{
  int8_t rslt;

  dev.intf = BME280_I2C_INTF;
  dev.read = bme_read;
  dev.write = bme_write;
  dev.delay_us = bme_delay;
  dev.intf_ptr = NULL;

  rslt = bme280_init(&dev);
  if (rslt != BME280_OK) return rslt;

  settings.osr_h  = BME280_OVERSAMPLING_1X;
  settings.osr_p  = BME280_OVERSAMPLING_1X;
  settings.osr_t  = BME280_OVERSAMPLING_1X;
  settings.filter = BME280_FILTER_COEFF_OFF;
  rslt = bme280_set_sensor_settings(BME280_SEL_ALL_SETTINGS, &settings, &dev);
  if (rslt != BME280_OK) return rslt;

  return bme280_set_sensor_mode(BME280_POWERMODE_NORMAL, &dev);
}

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
  HAL_Delay(500);
  dbg("--- Weather station start ---");

  if (bme_init_sensor() != BME280_OK)
  {
    dbg("BME280 init FAILED");
    while (1)                              /* fast LED blink = sensor error */
    {
      HAL_GPIO_TogglePin(STATUS_PORT, STATUS_PIN);
      HAL_Delay(100);
    }
  }
  dbg("BME280 OK");

  wifi_ready = wifi_join();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    if (!mqtt_ready)
    {
      if (!wifi_ready) wifi_ready = wifi_join();
      if (wifi_ready) mqtt_ready = mqtt_connect();

      if (!mqtt_ready)
      {
        mqtt_fails++;
        if (mqtt_fails >= 3)
        {
          wifi_ready = 0;                  /* force a fresh Wi-Fi join */
          mqtt_fails = 0;
        }
        HAL_Delay(3000);
        continue;
      }
      mqtt_fails = 0;
    }

    if (bme280_get_sensor_data(BME280_ALL, &comp, &dev) == BME280_OK)
    {
      char json[96];
      snprintf(json, sizeof(json), "{\"t\":%.2f,\"h\":%.2f,\"p\":%.2f}",
               comp.temperature, comp.humidity, comp.pressure / 100.0f);
      dbg("Reading: %s", json);

      if (!mqtt_publish(json))
      {
        mqtt_ready = 0;                    /* reconnect on the next loop */
      }
    }
    else
    {
      dbg("Sensor read FAILED");
    }

    HAL_GPIO_TogglePin(STATUS_PORT, STATUS_PIN);
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
  * @brief USART1 Initialization Function (debug, 9600 baud for the Arduino bridge)
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
  huart1.Init.BaudRate = 9600;
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
  * @brief USART2 Initialization Function (Wemos AT link, 115200 baud)
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
  {
    GPIO_InitTypeDef gi = {0};
    __HAL_RCC_GPIOC_CLK_ENABLE();
    HAL_GPIO_WritePin(STATUS_PORT, STATUS_PIN, GPIO_PIN_SET);   /* LED off */
    gi.Pin   = STATUS_PIN;
    gi.Mode  = GPIO_MODE_OUTPUT_PP;
    gi.Pull  = GPIO_NOPULL;
    gi.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(STATUS_PORT, &gi);
  }
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

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