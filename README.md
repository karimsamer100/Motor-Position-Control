<div align="center">

<br>

```
███╗   ███╗ ██████╗ ████████╗ ██████╗ ██████╗     ██████╗ ██████╗ ███╗   ██╗████████╗██████╗  ██████╗ ██╗     
████╗ ████║██╔═══██╗╚══██╔══╝██╔═══██╗██╔══██╗   ██╔════╝██╔═══██╗████╗  ██║╚══██╔══╝██╔══██╗██╔═══██╗██║     
██╔████╔██║██║   ██║   ██║   ██║   ██║██████╔╝   ██║     ██║   ██║██╔██╗ ██║   ██║   ██████╔╝██║   ██║██║     
██║╚██╔╝██║██║   ██║   ██║   ██║   ██║██╔══██╗   ██║     ██║   ██║██║╚██╗██║   ██║   ██╔══██╗██║   ██║██║     
██║ ╚═╝ ██║╚██████╔╝   ██║   ╚██████╔╝██║  ██║   ╚██████╗╚██████╔╝██║ ╚████║   ██║   ██║  ██║╚██████╔╝███████╗
╚═╝     ╚═╝ ╚═════╝    ╚═╝    ╚═════╝ ╚═╝  ╚═╝    ╚═════╝ ╚═════╝ ╚═╝  ╚═══╝   ╚═╝   ╚═╝  ╚═╝ ╚═════╝ ╚══════╝
```

### `[ DC Motor · Angular Position · PID · Arduino ]`

<br>

[![Arduino](https://img.shields.io/badge/Arduino-Uno-00979D?style=flat-square&logo=arduino)](https://www.arduino.cc/)
[![C++](https://img.shields.io/badge/Language-C%2B%2B-00599C?style=flat-square&logo=cplusplus)](https://isocpp.org/)
[![Python](https://img.shields.io/badge/Analysis-Python_3-3776AB?style=flat-square&logo=python)](https://www.python.org/)
[![Status](https://img.shields.io/badge/Status-✅_Complete-brightgreen?style=flat-square)]()
[![University](https://img.shields.io/badge/ASU-CSE481-8B0000?style=flat-square)]()

<br>

> *"Given a target angle, the motor gets there - fast, smooth, and stays there."*

<br>

</div>

---

##  Abstract

This project delivers a **robust closed-loop PID position controller** running on an Arduino Uno at 50 Hz. A geared DC motor with integrated quadrature encoder is commanded to reach angular setpoints (30°, 180°, 360°) from rest. The controller overcomes real-world nonlinearities - specifically **gear stiction** and **integral windup** - through a layered compensation strategy, achieving fast settling (<1.5 s) with zero steady-state error.

| | |
|---|---|
| **University** | Ain Shams University · Faculty of Engineering |
| **Course** | CSE481 - Control Systems |
| **Team** | Karim Samer · Tarek Mohammed |
| **Platform** | Arduino Uno Rev3 · GA25-370 Motor · L298N |

---

##  Performance at a Glance

```
  Setpoint │ Settling Time │  Overshoot  │ Steady-State Error
  ─────────┼───────────────┼─────────────┼───────────────────
     30°   │    ~640 ms    │   ~6° (≈20%)│       0°   
    180°   │    ~960 ms    │  ~29° 16%   │    ≤ 0.71° 
    360°   │   ~1460 ms    │  ~32°  9%   │       0°   
```

> Settling criterion: `|error| < 1°` · Sample rate: 50 Hz · Supply: 12V

---

##  Hardware Architecture

```
  ╔═══════════════════════════════════════════════════════════════╗
  ║                    SYSTEM BLOCK DIAGRAM                       ║
  ╠═══════════════════════════════════════════════════════════════╣
  ║                                                               ║
  ║   θ_ref ──►(+)──► ┌─────────┐  u(V)  ┌────────┐  ┌───────┐    ║
  ║            (-)    │   PID   ├────────►│ L298N  ├─►│ Motor │   ║
  ║             ▲     │Controller│        │H-Bridge│  │GA25   │   ║
  ║             │     └─────────┘        └────────┘  └───┬───┘    ║
  ║             │                                         │       ║
  ║             │     ┌──────────────────┐                │       ║
  ║             └─────┤ Encoder Feedback │◄───────────────┘       ║
  ║                   │  925 PPR · ISR   │                        ║
  ║                   └──────────────────┘                        ║
  ╚═══════════════════════════════════════════════════════════════╝
```

### Bill of Materials

| # | Component | Model | Role |
|---|---|---|---|
| 1 | Microcontroller | Arduino Uno Rev3 (ATmega328P) | PID computation + I/O |
| 2 | DC Motor + Encoder | GA25-370 · 12V · 130 RPM | Actuator + feedback |
| 3 | Motor Driver | L298N Dual H-Bridge | Bidirectional PWM drive |
| 4 | Power Supply | 12V DC Adapter | Motor power |
| 5 | Mechanical Aid | 3D Printed Pointer (PLA) | Angle visualization |

---

##  Controller Design

### PID Architecture

The controller runs in a **fixed 20 ms timer loop** (50 Hz). Each cycle:

```
1. Read encoder count (atomic, interrupt-safe)
2. Convert counts → angle:  θ = (count / 925) × 360°
3. Compute error:            e = θ_ref − θ
4. Update PID terms
5. Apply stiction compensation
6. Clamp to [−12V, +12V]
7. Map voltage → PWM [0–255]
8. Drive motor or brake
```

### Tuned PID Gains

```cpp
// ── Final Tuned Parameters ─────────────────────────────────
const float Kp    = 0.04;   // Proportional gain
const float Ki    = 0.90;   // Integral gain
const float Kd    = 0.02;   // Derivative gain
const int   Ts    = 20;     // Sample time [ms]
const float Vmax  = 12.0;   // Voltage saturation [V]
const float PPR   = 925.0;  // Encoder pulses per revolution
// ────────────────────────────────────────────────────────────
```

###  Nonlinearity Solutions

**Problem 1 - Gear Stiction**

The GA25-370's gearbox requires ~2.5V to break static friction. Below this threshold, the controller commands motion but nothing happens - the integral keeps winding up, causing a violent lurch when friction finally breaks.

**Solution: Feedforward Stiction Kick**

```cpp
float stiction_voltage = 2.5; // V - minimum to overcome static friction
if      (controlVoltage >  0.05) controlVoltage += stiction_voltage;
else if (controlVoltage < -0.05) controlVoltage -= stiction_voltage;
```

---

**Problem 2 - Integral Windup**

During the stiction phase, error persists but the motor doesn't move. The integral accumulates to dangerous levels, causing massive overshoot when motion begins. Diagnosed clearly in `PID_Trail_1.csv` (settling time > 10 seconds).

**Solution A: Zero-Crossing Reset**

```cpp
// When we cross the target, instantly wipe the integral memory
if ((error > 0 && previous_error < 0) || (error < 0 && previous_error > 0)) {
    integral = 0;
}
```

**Solution B: Integral Clamping**

```cpp
// Cap the integral so it can never contribute more than 3V equivalent
float max_integral_memory = 3.0 / Ki;
integral = constrain(integral, -max_integral_memory, max_integral_memory);
```

**Solution C: Deadband + Dynamic Braking**

```cpp
if (abs(error) < 1.0) {
    // Dynamic braking: both pins HIGH at full PWM
    digitalWrite(IN1_PIN, HIGH);
    digitalWrite(IN2_PIN, HIGH);
    analogWrite(ENA_PIN, 255);
    // Integral NOT updated - prevents deadband windup
}
```

---

##  Repository Map

```
motor-position-control/
│
├──   arduino/
│   ├── PID/
│   │   └── PID.ino                  ← MAIN - final controller
│   ├── ParameterEstimation/
│   │   └── ParameterEstimation.ino  ← Open-loop data logger (step/pulse)
│   └── PPREstimation/
│       └── PPREstimation.ino        ← Encoder PPR calibration
│
├──  Data/
│   ├── raw/
│   │   ├── StepResponse.csv         ← Full-throttle step response
│   │   └── PulseResponse.csv        ← On/off pulse for time constants
│   └── pid_trials/
│       ├── PID_Trail_1.csv          ← Failed trial - integral windup demo
│       ├── PID_Success_30_Deg.csv   ←  Tuned - 30° setpoint
│       ├── PID_Success_180_Deg.csv  ←  Tuned - 180° setpoint
│       └── PID_Success_360_Deg.csv  ←  Tuned - 360° setpoint
│
├──  scripts-notebooks/
│   ├── PID_Trail_1.ipynb            ← Windup analysis & diagnosis
│   ├── PID_Response_Analysis.ipynb  ← Final results comparison plot
│   └── main.py                      ← CSV preprocessing utility
│
├──   3d-designs/
│   └── Pointer Design.md            ← PLA pointer specs (4mm shaft, 2.8mm flat)
│
└──  hardware/
    ├── components_list.md
    └── wiring_connections.md
```

---

##  Replication Guide

### Step 1 - Wire the Hardware

```
Arduino D2  ──►  Encoder Channel A  (CHANGE interrupt)
Arduino D3  ──►  Encoder Channel B
Arduino D5  ──►  L298N ENA          (PWM speed)
Arduino D6  ──►  L298N IN1          (direction)
Arduino D7  ──►  L298N IN2          (direction)
Arduino 5V  ──►  Encoder VCC
Arduino GND ──►  Encoder GND + L298N GND
12V Supply  ──►  L298N 12V terminal
```

### Step 2 - Flash the Controller

```cpp
// arduino/PID/PID.ino - change this line for your target:
float setpoint = 90;   // ← set your desired angle in degrees
```

Upload via Arduino IDE (Board: Uno · Baud: 115200)

### Step 3 - Log & Analyze

Open Serial Monitor at 115200 baud. CSV data streams in real-time:

```
Time(ms), Setpoint, Angle, ControlVoltage, PWM, Error, Integral, Derivative
20, 90.00, 0.00, 6.09, 129, 90.00, 1.80, 4500.00
40, 90.00, 1.41, 6.19, 131, 88.59, 3.57, -70.59
...
```

Copy output to `Data/pid_trials/` and run:

```bash
pip install pandas matplotlib jupyter
jupyter notebook scripts-notebooks/PID_Response_Analysis.ipynb
```

---

##  Tuning Journey

```
TRIAL 1 (PID_Trial_1.csv)
  Kp=0.04  Ki=0.9  Kd=0.02  - No anti-windup, no stiction comp.
  Result: Settling > 10 seconds. Windup clearly visible in integral plot.
  Root cause: Integral accumulated to −26 units during stiction phase.
       │
       ▼  Added stiction kick (2.5V) + zero-crossing reset + clamping
       │
FINAL SUCCESS
  Same gains - with all 3 compensation layers
  Result: Settling < 1 second across all setpoints. Clean step response.
```

---

<div align="center">

---

*Built with 🔧 at Ain Shams University · CSE481 Control Systems · Spring 2026*

**Karim Samer · Tarek Mohammed**

[![ASU](https://img.shields.io/badge/Ain_Shams_University-Engineering-8B0000?style=for-the-badge)]()

</div>
