# CAT3 – Timer-Based LED Pattern Generator and Button Interrupt Controller

|Field|Details|
|-|-|
|**EXP**|CAT3 – Continuous Lab Evaluation-3 (25CS2741 Fundamentals of Internet of Things + Lab)|
|**DATE**|27/07/2026|
|**NAME OF EXP**|Timer-Based LED Pattern Generator and Button Interrupt Controller|
|**HARDWARE SETUP / PLATFORM**|AT89S52 microcontroller development board (8051 tool chain), Embedded C (Keil), register-level bare-metal, multimeter for hardware debug|
|**TASK NO**|Single activity: register-level four-state LED controller with debounced push-button|

## Aim

To design and implement a register-level, bare-metal embedded C program for an AT89S52 microcontroller development board to control a four-state LED system using direct memory-mapped register access and switch debouncing, changing modes upon successive push-button presses, without using Arduino APIs, HAL libraries or RTOS services.

## Hardware Setup

**Platform:** AT89S52 (8051 family) development board

**Components:**

* 4 × LEDs (active high)
* 4 × 1 kΩ resistors (one in series with each LED)
* 1 × push-button (SW1)
* Breadboard and connecting wires
* Power supply and digital multimeter

**Pin connections:**

|Pin|Connected to|Direction|
|-|-|-|
|P1.0|LED1 (via 1 kΩ) → GND|Output|
|P1.1|LED2 (via 1 kΩ) → GND|Output|
|P1.2|LED3 (via 1 kΩ) → GND|Output|
|P1.3|LED4 (via 1 kΩ) → GND|Output|
|P2.0|SW1, read as the button input (pressed = 0)|Input|
|P2.7|SW1 other terminal, driven low (0) in code as the ground reference|Output|

## Circuit Diagram

!\[Circuit Diagram](circuit.jpeg)

## Core Logic / Pseudocode

Each valid press of SW1 moves the system to the next mode.

|Button press|Mode|
|-|-|
|Power-up|Mode 1 – Running LED (starts automatically, no press needed)|
|Press 1|Mode 2 – Binary counter|
|Press 2|Mode 3 – Alternating pattern|
|Press 3|Mode 4 – All LEDs flash|
|Press 4|Back to Mode 1 – Running LED|

```
setup: P2.7 = 0 (reference for the button), all LEDs OFF, count = 1   // Mode 1 starts automatically

loop forever:
    if button reads 0 (pressed):
        delay                                  // debounce
        if button still reads 0:
            count = count + 1
            if count > 4: count = 1            // after Mode 4 the next press returns to Mode 1
            wait until button is released
            delay                              // release debounce

    switch (count):
        1: Running LED      - LED1 ON, delay, OFF; LED2 ON, delay, OFF; LED3 ...; LED4 ...
        2: Binary counter   - for n = 0 to 15: P1 = n, delay
        3: Alternating      - LED1+LED3 ON (LED2+LED4 OFF), delay; then LED2+LED4 ON, delay
        4: All flash        - all LEDs ON, delay; all LEDs OFF, delay
```

**Debouncing:** the button is read, a software delay is added, and it is read again. Only if it is still pressed is the mode advanced. The program then waits for the button to be released and adds another delay, so one physical press produces exactly one state transition.

**Timing and input method:** no hardware timer and no interrupt is used. Timing comes from a nested software delay loop (250 × 250 iterations), and the button is polled once at the top of each pass through the main loop.

**Source file:** `ch3.c` (uses only `<reg52.h>` for the register definitions, with direct writes to P1 and P2).

## Inference

The four LEDs on P1.0–P1.3 were controlled by writing directly to the P1 port, and the push-button on P2.0 was read directly from the P2 port, with P2.7 held low as its reference. Successive presses of SW1 cycled the system through the running LED, binary counter, alternating and all-flash modes. The software debounce (delay, re-check, wait for release) made each physical press count exactly once. Because the button is only checked between pattern cycles, a press has to be held until the current pattern finishes. The program uses only register-level access, with no Arduino APIs, HAL libraries or RTOS.

