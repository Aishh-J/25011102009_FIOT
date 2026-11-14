# EXPERIMENT DETAILS

**EXP:** FPIoT LAB EVALUATION - CAT 12

**DATE:** <23/10/2026>

**NAME OF EXP:** STM32 On-Board LED Blink Pattern

**HARDWARE SETUP / PLATFORM:** CubeMX + Keil MDK + STM32 Nucleo-64 Development Board (NUCLEO-F401RE) + On-board LED (LD2)

**TASK NO:** Task A – GPIO Output (on-board LED blink pattern using HAL\_Delay)

\---

## 1\. Circuit Diagram

This task uses only the on-board user LED (LD2) of the Nucleo board, so no external components or wiring are needed. The board is powered and programmed through the USB (ST-LINK) connector.

* Platform: STM32 Nucleo-64 board (MB1136), 84 MHz system clock from the internal oscillator (HSI) through the PLL
* Output: on-board user LED LD2 on PA5 (HIGH = ON, LOW = OFF)
* Power Supply: 5V via the ST-LINK USB connector

!\[Circuit Diagram](circuit.png)

\---

## 2\. Core Logic / Pseudocode / Code Base

**Working Principle:**
The LED LD2 on PA5 is driven by a repeating blink pattern: three slow blinks (1 second ON, 1 second OFF), a pause, three faster blinks (0.5 second ON, 0.5 second OFF) and another pause. The LED is toggled with `HAL\_GPIO\_TogglePin()` and the timing is produced with `HAL\_Delay()`. The LED is OFF at start-up.

**Blink Pattern:**

|Phase|LED ON time|LED OFF time|Repeats|
|-|-|-|-|
|Three slow blinks|1000 ms|1000 ms|3 times|
|Pause|–|1000 ms|once|
|Three fast blinks|500 ms|500 ms|3 times|
|Pause|–|1000 ms|once|

The whole pattern then repeats forever.

**Pseudocode:**

1. Initialize the HAL library and system clock (HSI + PLL, 84 MHz).
2. Configure PA5 (LD2) as a push-pull output and set it LOW (LED off).
3. Repeat continuously:

   1. Repeat 3 times: toggle LED (ON), wait 1000 ms, toggle LED (OFF), wait 1000 ms.
   2. Wait 1000 ms (pause).
   3. Repeat 3 times: toggle LED (ON), wait 500 ms, toggle LED (OFF), wait 500 ms.
   4. Wait 1000 ms (pause).

**Code Base:**
The program is written in C using the STM32 HAL library and generated with STM32CubeMX.

* `LD2\_Pin` and `LD2\_GPIO\_Port` are the CubeMX user labels for PA5, the on-board LED.
* `HAL\_GPIO\_TogglePin(LD2\_GPIO\_Port, LD2\_Pin)` switches the LED between ON and OFF.
* `HAL\_Delay(ms)` creates the ON, OFF and pause times using the millisecond SysTick time base.
* `MX\_GPIO\_Init()` configures PA5 as push-pull output (low speed), starting LOW (LED off).
* `SystemClock\_Config()` runs the PLL from the internal HSI oscillator (16 MHz / 16 x 336 / 4 = 84 MHz), with AHB = 84 MHz, APB1 = 42 MHz, APB2 = 84 MHz and 2 flash wait states.
* The push-button B1 and USART2 are also configured by CubeMX, but they are not used in this task.

**Pin Summary:**

|Pin|Mode|Function|
|-|-|-|
|PA5 (LD2)|Output, push-pull|On-board LED (HIGH = ON, LOW = OFF)|

\---

## 3\. Inference

The on-board LED repeats a clear pattern of three slow blinks followed by three fast blinks, with pauses in between. Changing the delay values changes the blink speed, which shows how `HAL\_Delay()` is used to control timing. Since `HAL\_Delay()` blocks the processor, the microcontroller can do nothing else while the pattern runs, which is acceptable for this simple task but would be avoided in applications that must stay responsive (non-blocking timing with `HAL\_GetTick()` is used for that, as in the earlier tasks). This demonstrates GPIO output configuration with CubeMX, clock configuration and LED control using STM32 HAL functions.

