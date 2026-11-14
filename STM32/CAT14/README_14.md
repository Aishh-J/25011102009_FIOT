# EXPERIMENT DETAILS

**EXP:** FPIoT LAB EVALUATION - CAT 14

**DATE:** 28/10/2026

**NAME OF EXP:** BME280 Sensor Data Publishing over MQTT using ESP8266

**HARDWARE SETUP / PLATFORM:** CubeMX + Keil MDK + STM32 Development Board (STM32F103C8T6) + BME280 Sensor Module + ESP8266 Wi-Fi Module + MQTT Broker (broker.emqx.io) + Node-RED

**TASK NO:** BME280 readings sent as a JSON payload over Wi-Fi using MQTT (ESP8266 as the network interface)

\---

## 1\. Circuit Diagram

The STM32 reads temperature, humidity and pressure from the BME280 sensor over I2C and sends them to an MQTT broker through an ESP8266 Wi-Fi module, which is connected over UART and controlled with AT commands. The data can then be received and visualized in Node-RED with an `mqtt in` node.

* Microcontroller: STM32F103C8T6 development board (Blue Pill), 72 MHz system clock from the 8 MHz crystal (HSE) through the PLL (x9)
* BME280 sensor on I2C1 (100 kHz), I2C address 0x76 (SDO connected to GND)
* ESP8266 Wi-Fi module on USART2 at 115200 baud (8N1), used through the ESP8266 and MQTT library (`ESPLib.h`)
* Onboard LED on PC13 (active-low) used to signal errors
* Power Supply: 5V via USB, sensor and ESP8266 on 3.3V

|BME280 pin|STM32 pin|
|-|-|
|VCC|3.3V|
|GND|GND|
|SCL|PB8 (I2C1\_SCL)|
|SDA|PB9 (I2C1\_SDA)|
|SDO|GND (address 0x76)|

|ESP8266 pin|Connection|
|-|-|
|VCC|Stable 3.3V|
|GND|Common GND|
|RX|STM32 USART2 TX|
|TX|STM32 USART2 RX|
|EN / CH\_PD|3.3V|

The ESP8266 is a 3.3V device and needs a stable supply, and all grounds must be connected together.

!\[Circuit Diagram](circuit.jpg)

\---

## 2\. Core Logic / Pseudocode / Code Base

**Working Principle:**
After start-up the program first sets up the BME280 sensor, so a wiring fault is found before any time is spent on Wi-Fi. It then checks that the STM32 can really hear the ESP8266 by sending a raw `AT` command and waiting for `OK`. After that it initializes the ESP8266 in station mode, connects to the Wi-Fi network, opens a TCP connection to the MQTT broker and connects as an MQTT client. In the main loop the sensor is read once per second, the values are converted into a JSON string and the string is published to the MQTT topic with QoS 0.

**MQTT Settings:**

|Parameter|Value|
|-|-|
|Broker|broker.emqx.io|
|Port|1883 (standard unencrypted MQTT port)|
|Client ID|stm32bme01|
|Topic|stm32/bme280/goofy|
|QoS|0|
|Keep-alive|500|
|Connect flag|CleanSession|
|Publish period|1000 ms|

The Wi-Fi name and password are set in the `WIFI\_SSID` and `WIFI\_PASS` defines at the top of `main.c` (they are not listed here).

**JSON Payload Format:**

```
{"temperature":28.45,"humidity":62.30,"pressure":1008.72}
```

Temperature is in degrees C, humidity in %RH and pressure in hPa, each with 2 decimal places.

**Sensor Settings (Bosch BME280 driver):**

|Parameter|Value|
|-|-|
|Temperature oversampling|1x|
|Pressure oversampling|1x|
|Humidity oversampling|1x|
|IIR filter|Off|
|Standby time|1000 ms|
|Sensor mode|Normal (continuous measurement)|

**Pseudocode:**

1. Initialize the HAL library, system clock (HSE + PLL, 72 MHz), GPIO, I2C1, USART1 and USART2.
2. Set up the BME280 (`bme280\_setup()`):

   1. Link the driver to the STM32 I2C read, write and delay functions.
   2. Read the chip ID from register 0xD0 and check that it is 0x60.
   3. Initialize the sensor with `bme280\_init()`, configure oversampling, filter and standby time, and set normal mode.
   4. If any step fails, stop and blink the LED (error code stored in `dbg\_error`).
3. Wait 1.5 s, then send `AT` to the ESP8266 up to 6 times until `OK` is received. If there is no reply, stop with an error.
4. Initialize the ESP8266 in station mode with `ESP8266\_Init()`.
5. Connect to the Wi-Fi network with `ESP8266\_wifiConnect()`.
6. Open a TCP connection to the broker with `ESP8266\_portConnect("TCP", broker, port)`.
7. Set keep-alive, connect flag and client ID, then connect with `MQTT\_connectBroker()`.
8. Repeat continuously:

   1. Read the compensated data with `bme280\_get\_sensor\_data()`.
   2. If the read is successful, convert pressure from Pa to hPa, build the JSON string with `snprintf()` and publish it with `MQTT\_publishTopic()`.
   3. Increase `publish\_count` and wait 1000 ms.

**Code Base:**
The program is written in C using the STM32 HAL library, the Bosch BME280 sensor API (`bme280.h`) and the ESP8266 and MQTT library (`ESPLib.h`).

* `user\_i2c\_read()` and `user\_i2c\_write()` use `HAL\_I2C\_Mem\_Read()` and `HAL\_I2C\_Mem\_Write()` (100 ms timeout) and `user\_delay\_us()` uses `HAL\_Delay()`, so the Bosch driver can use the STM32 I2C bus. The compensation equations are done inside the driver.
* `bme280\_setup()` contains the complete sensor start-up: chip ID check, init, settings and normal mode.
* `esp\_raw\_at\_test()` throws away old bytes, sends `AT\\r\\n` and stores the reply in `esp\_rx`. A healthy reply is 10 bytes (`AT`, `OK` with line endings).
* `app\_error()` stores the error code in `dbg\_error`, and blinks the PC13 LED every 200 ms forever.
* Error codes: 1 = I2C read failed, 2 = wrong chip ID, 3 = sensor init failed, 4 = sensor settings failed, 5 = set mode failed, 6 = no reply from ESP8266.
* `stage` shows the progress in the debugger Watch window: 0 = sensor setup, 1 = sensor ready, 2 = ESP8266 answers, 3 = ESP8266 initialized, 4 = Wi-Fi connected, 5 = TCP connected to broker, 6 = MQTT connected, 7 = publishing.
* `temperature`, `pressure`, `humidity`, `chip\_id`, `rslt` and `publish\_count` are `volatile` global variables, so they can be added to the Watch window.
* A compile-time check makes sure the topic is short enough for the buffer used inside the ESP library.
* `SystemClock\_Config()` sets HSE on with PLL x9 as the system clock (72 MHz), with APB1 = 36 MHz and 2 flash wait states.

**Viewing the Output in Node-RED:**
Add an `mqtt in` node (Network category) and set the server to `broker.emqx.io` port 1883 and the topic to `stm32/bme280/goofy`. Connect it to a `debug` node (and to gauge or chart nodes through a `json` node) to see and visualize the three values.

\---

## 3\. Inference

The STM32 reads the BME280 sensor over I2C and publishes the temperature, humidity and pressure once per second as a JSON message to a public MQTT broker through the ESP8266, and Node-RED receives the same message by subscribing to the topic. Using the Bosch driver for the sensor and the ESP and MQTT library for the network means the program only has to connect the building blocks: I2C functions for the driver, AT-command based Wi-Fi and TCP set-up, and one publish call per reading. Testing each stage in order (sensor, ESP8266 reply, Wi-Fi, TCP, MQTT) with the `stage` and `dbg\_error` variables and the error LED makes it easy to find which part of the chain has a problem. MQTT QoS 0 is used, so each message is sent once without confirmation, which is enough for periodic sensor data because a new value follows every second. This demonstrates I2C sensor interfacing, UART communication with a Wi-Fi module, the MQTT publish model and building a JSON payload for an IoT dashboard.

