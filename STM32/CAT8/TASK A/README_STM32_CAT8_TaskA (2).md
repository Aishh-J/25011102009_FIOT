# EXPERIMENT DETAILS

**EXP:** CONTINUOUS LAB EVALUATION-8

**DATE:** 18/09/2026

**NAME OF EXP:** STM32 GPIO Bring-Up and Switch-Controlled LED

**HARDWARE SETUP / PLATFORM:** CubeMX + Keil MDK + STM32 Development Board + Push-Button + LED

---

## 1. Circuit Diagram

The experiment uses a push-button as input and an LED as output. The LED follows the switch state.

* Microcontroller: STM32 development board
* Input: push-button on PA6 to GND, using the internal pull-up (pressed = LOW)
* Output: onboard LED on PC13 (active low, LOW = ON)
* Power Supply: 5V via USB

![Circuit Diagram](cat8_taskA_circuit.png)

---

## 2. Core Logic / Pseudocode / Code Base

**Working Principle:**
PA6 is read continuously. The LED on PC13 is turned on while the button is pressed and off while it is released.

**Pseudocode:**

1. Initialize the HAL library and system clock.
2. Configure PC13 as a push-pull output.
3. Configure PA6 as an input with pull-up.
4. Repeat continuously:
   1. Read PA6.
   2. If PA6 is LOW (pressed), set PC13 LOW (LED on).
   3. Otherwise, set PC13 HIGH (LED off).

**Code Base:**
The program is written in C using the STM32 HAL library and generated with STM32CubeMX.

* `HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6)` returns GPIO_PIN_RESET when the button is pressed.
* `HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, state)` sets the LED. GPIO_PIN_RESET turns the LED on (active low).
* `MX_GPIO_Init()` configures PC13 as output and PA6 as input with pull-up.

---

## 3. Inference

The LED state follows the push-button state directly, demonstrating GPIO input reading and output control using STM32 HAL functions, including the active-low LED on PC13.
