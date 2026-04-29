# Motor, Encoder, and Driver Wiring Connections

## Overview
This document describes the wiring connections between:
- DC Motor with Encoder
- L298N Motor Driver
- Arduino Uno
- External 12V Power Supply

---

## Motor + Encoder Wiring

The motor has 6 wires, each with a specific function:

| Wire Color |     Function      | Connection|
|------------|-------------------|------------|
| Red        | Motor Power (+)   | L298N OUT2 |
| White      | Motor Power (-)   | L298N OUT1 |
| Blue       | Encoder VCC (+)   | Arduino 5V |
| Black      | Encoder GND (-)   | Arduino GND|
| Yellow     | Encoder Channel A | Arduino D3 |
| Green      | Encoder Channel B | Arduino D2 |

---

## Arduino to L298N Connections

| Arduino Pin | L298N Pin | Function |
|------------|----------|----------|
| D5 | ENA | PWM Speed Control |
| D6 | IN1 | Motor Direction Control |
| D7 | IN2 | Motor Direction Control |
| GND | GND | Common Ground |

---

## Power Connections

| Component | Connection |
|----------|-----------|
| Power Supply (+12V) | L298N 12V |
| Power Supply GND | L298N GND |

---

## Important Notes

- All grounds are connected together (common ground):
  - Arduino GND
  - L298N GND
  - Power supply GND
  - Encoder GND

- The motor direction can be reversed by swapping:
  - OUT1 and OUT2 connections

- Encoder operates at:
  - 3.3V – 5V (powered from Arduino)

- Encoder resolution:
  - 11 pulses per motor shaft revolution
  - Total counts depend on gearbox reduction ratio

---

## System Flow

```text
Arduino → L298N → Motor → Encoder → Arduino (feedback)