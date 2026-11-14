# CAT4 – Human-Machine Interface using Matrix Keypad, LEDs, and Seven-Segment Display

|Field|Details|
|-|-|
|**EXP**|CAT4 – Continuous Lab Evaluation-4 (25CS2741 Fundamentals of Internet of Things + Lab)|
|**DATE**|03/08/2026|
|**NAME OF EXP**|Human-Machine Interface using Matrix Keypad, LEDs, and Seven-Segment Display|
|**HARDWARE SETUP / PLATFORM**|AT89S52 development board (8051 tool chain), Embedded C (Keil), 4×4 matrix keypad, seven-segment display, 4 LEDs, multimeter for hardware debug|
|**TASK NO**|Single activity: keypad-controlled LED functions with seven-segment feedback (no sub-tasks)|

## Aim

To interface a 4×4 matrix keypad, a seven-segment display and 4 LEDs with the AT89S52 microcontroller and perform different LED operations based on the key pressed. Whenever a valid key is pressed, its value is shown on the seven-segment display and the corresponding LED function is executed, without using Arduino APIs, HAL libraries or RTOS services.

## Hardware Setup

**Platform:** AT89S52 (8051 family) development board

**Components:**

* 4 × LEDs (active high)
* 4 × 1 kΩ resistors (one in series with each LED)
* 4×4 matrix keypad
* Seven-segment display
* Breadboard and jumper wires
* USB cable, power supply and digital multimeter

**Pin connections:**

|Pin|Connected to|Direction|
|-|-|-|
|P3.2|LED1 (via 1 kΩ) → GND|Output|
|P3.3|LED2 (via 1 kΩ) → GND|Output|
|P3.4|LED3 (via 1 kΩ) → GND|Output|
|P3.5|LED4 (via 1 kΩ) → GND|Output|
|P2.7, P2.6, P2.5, P2.4|Keypad scan lines (driven low one at a time)|Output|
|P2.3, P2.2, P2.1, P2.0|Keypad sense lines (read to detect a pressed key)|Input|
|P0.0 – P0.6|Seven-segment display segments a – g (the program's segment codes are for a common-cathode display and assume P0.0 = a … P0.6 = g)|Output|

## Circuit Diagram

!\[Circuit Diagram](circuit.jpeg)

## Core Logic / Pseudocode

Each valid key press shows the key value on the seven-segment display and runs the matching LED function.

|Key|Seven-segment display|LED operation|
|-|-|-|
|1|1|LED1 ON|
|2|2|LED2 ON|
|3|3|LED3 ON|
|4|4|LED4 ON|
|5|5|Running LED pattern|
|6|6|Alternate LEDs (1010 ↔ 0101)|
|7|7|Binary counter (0–15)|
|8|8|All LEDs blink|
|9|9|All LEDs OFF|
|0|0|Reset (LEDs OFF, display 0)|

**Keypad map** (scan line driven low, sense line that goes low):

|Scan line|P2.3|P2.2|P2.1|P2.0|
|-|-|-|-|-|
|P2.7|1|2|3|4|
|P2.6|5|6|7|8|
|P2.5|9|0|A|B|
|P2.4|C|D|E|F|

Keys A – F are scanned but have no function.

**Functions in the program:**

* `keypad\\\\\\\_scan()` waits until a valid key is pressed and returns the key.
* `display\\\\\\\_digit(value)` writes the segment code for the digit to P0.
* `external\\\\\\\_led(pattern)` writes the lower 4 bits of the pattern to P3.2 – P3.5 without changing the other P3 pins.
* `running\\\\\\\_led()` runs LED1 → LED4 three times (300 ms per step).
* `alternate\\\\\\\_led()` alternates 1010 and 0101 six times (400 ms per step).
* `binary\\\\\\\_counter()` counts 0 to 15 on the LEDs (500 ms per step).
* `blink\\\\\\\_all()` blinks all LEDs five times (400 ms on, 400 ms off).
* `delay\\\\\\\_ms(ms)` is a software delay loop.
* The key-to-LED selection is a `switch` statement in `main()`.

```
setup: display shows 0, P2 = 0xFF, P3 = 0x00 (all LEDs OFF)

loop forever:
    key = keypad\\\\\\\_scan()                  // waits here until a key is pressed
    if key is '0' to '9':
        display\\\\\\\_digit(key)               // show the digit on the seven-segment display

    switch (key):
        '1': LED1 ON only
        '2': LED2 ON only
        '3': LED3 ON only
        '4': LED4 ON only
        '5': running\\\\\\\_led()
        '6': alternate\\\\\\\_led()
        '7': binary\\\\\\\_counter()
        '8': blink\\\\\\\_all()
        '9': all LEDs OFF
        '0': all LEDs OFF and display 0  // reset
        other keys (A to F): ignored

keypad\\\\\\\_scan():
    loop forever:
        for each scan line P2.7, P2.6, P2.5, P2.4:
            drive that line low and the other lines high
            if any sense line P2.3 to P2.0 reads low:
                delay 20 ms              // debounce
                read the sense lines again
                if a line is still low:
                    wait until the key is released
                    return the key for this scan line and sense line
```

**Debouncing:** a key press is detected when a sense line goes low. The program waits 20 ms, reads the lines again and accepts the key only if it is still pressed. It then waits for the key to be released, so one press produces exactly one action.

**Timing and input method:** no hardware timer and no interrupt is used. Delays come from software loops, and the keypad is polled. The keypad is not scanned while a pattern (keys 5 – 8) is running.

**Source file:** `ch4.c`

## Inference

The 4×4 keypad on P2 was scanned by driving one of P2.7 – P2.4 low at a time and reading P2.3 – P2.0. A 20 ms debounce and a wait for release made each key press count once. Each digit key was shown on the seven-segment display through P0 using a lookup table of segment codes, and the matching LED function on P3.2 – P3.5 was executed: a single LED for keys 1 – 4, and the running, alternating, binary-counter and blink patterns for keys 5 – 8. Key 9 turned all LEDs off, and key 0 reset the LEDs and the display. The program uses only register-level access, with no Arduino APIs, HAL libraries or RTOS. Because the keypad is scanned only between actions, keys are not read while a pattern is running.

