# Capacitive Displacement Sensor – Electronic Ruler

An experimental contactless displacement measurement system based on a parallel-plate capacitive sensor, an RC charging circuit, an LM358 operational amplifier, and an ESP32 microcontroller.

The system estimates the distance between two conductive plates by measuring the capacitor charging time. The measured capacitance is then converted into displacement using a piecewise cubic calibration model. An I2C LCD displays the capacitance and estimated distance in real time.

---

## 1. Project Overview

This project focuses on developing a low-cost electronic ruler using the variation of capacitance between two parallel conductive plates.

The sensing element consists of two PCB electrodes:
- One fixed electrode.
- One movable electrode.

Changing the gap between the electrodes changes the capacitance. An electronic interface converts this variation into a measurable time interval, which is processed by an ESP32 to estimate the displacement.

The final measurement is displayed on a 16×2 I2C LCD.

### Main Features

- Contactless capacitive displacement sensing.
- RC charging-time measurement.
- LM358-based threshold detection.
- ESP32-based timing and signal processing.
- Experimental calibration using PCHIP piecewise cubic interpolation.
- Real-time LCD measurement display.
- Serial Monitor output for debugging and data collection.

---

## 2. Project Objectives

The main objectives are:

- Design and construct a parallel-plate capacitive displacement sensor.
- Develop an electronic interface for capacitance measurement.
- Convert capacitance variations into RC charging-time variations.
- Process the measured time using an ESP32.
- Establish an experimental relationship between capacitance and displacement.
- Implement a calibration model to estimate the distance.
- Display the results on an LCD.
- Evaluate the measurement behavior under different experimental conditions.

---

## 3. Operating Principle

### 3.1 Capacitive Sensing Principle

The sensing element operates on the principle of a parallel-plate capacitor.

Its capacitance is approximately described by:

$$
C = \frac{\varepsilon_0 \varepsilon_r A}{d}
$$

Where:

- $C$: Capacitance (F).
- $\varepsilon_0$: Vacuum permittivity (F/m).
- $\varepsilon_r$: Relative permittivity of the dielectric.
- $A$: Effective overlapping electrode area (m²).
- $d$: Distance between the electrodes (m).

For an air dielectric:

$$
\varepsilon_r \approx 1
$$

The capacitance increases when the distance between the electrodes decreases. Conversely, increasing the gap reduces the capacitance.

### 3.2 RC Charging-Time Measurement

The capacitance is measured indirectly through an RC charging circuit.

The capacitor voltage during charging is:

$$
V_C(t) = V_S \left(1-e^{-\frac{t}{RC}}\right)
$$

The time constant is:

$$
\tau = RC
$$

The LM358-based threshold detection stage monitors the capacitor voltage and generates a digital transition when the voltage crosses the reference threshold.

For a threshold equal to half the charging supply:

$$
V_C(t)=\frac{V_S}{2}
$$

Substituting into the charging equation:

$$
\frac{V_S}{2}=V_S\left(1-e^{-\frac{t}{RC}}\right)
$$

Therefore:

$$
e^{-\frac{t}{RC}}=\frac{1}{2}
$$

Taking the natural logarithm:

$$
-\frac{t}{RC}=\ln\left(\frac{1}{2}\right)
$$

The charging time becomes:

$$
t=RC\ln(2)
$$

Thus, the capacitance can be estimated using:

$$
C=\frac{t}{R\ln(2)}
$$

Where:

- $t$: Measured charging time (s).
- $R$: Charging resistance (Ω).
- $C$: Capacitance (F).
- $\ln(2)\approx0.693147$.

**Important:** This simplified equation assumes that the comparator threshold is exactly half of the charging supply voltage. If the actual reference voltage differs, the general expression is:

$$
C=\frac{-t}{R\ln\left(1-\frac{V_{\mathrm{ref}}}{V_S}\right)}
$$

The actual circuit threshold and charging voltage should therefore be considered when evaluating measurement accuracy.

### 3.3 Measurement Sequence

The ESP32 performs the following operations during each measurement cycle:

1. Activates transistor Q2 to discharge the sensor capacitor.
2. Deactivates Q2 to start capacitor charging through the charging resistor.
3. Records the starting time using `micros()`.
4. Monitors the digital output from Q1.
5. Detects the threshold-crossing event.
6. Calculates the elapsed charging time.
7. Estimates the capacitance using the RC equation.
8. Converts capacitance into displacement using the calibration function.
9. Displays the results on the LCD and Serial Monitor.

---

## 4. Hardware Architecture

### 4.1 System Components

| Component | Description |
|---|---|
| ESP32 | Main controller and time measurement |
| LM358 | Analog threshold detection |
| 2× C1815 | Switching and signal-interface transistors |
| PCB electrodes | Capacitive sensing element |
| 100 kΩ resistor | RC charging resistor |
| 4.7 kΩ resistors | Transistor base resistors |
| 10 kΩ resistor | GPIO27 pull-up |
| 10 kΩ × 2 | Reference voltage divider |
| 16×2 I2C LCD | Measurement display |
| 100 nF capacitor | Supply decoupling |
| 5 V supply | LM358 power supply |

### 4.2 ESP32 Pin Configuration

| ESP32 Pin | Function |
|---|---|
| GPIO26 | Controls Q2 (capacitor discharge) |
| GPIO27 | Reads Q1 output |
| GPIO21 | I2C SDA |
| GPIO22 | I2C SCL |
| GND | Common ground |

### 4.3 Electronic Interface

The electronic interface is composed of three main stages:

**1. RC Charging Stage**

The sensor capacitor charges through a 100 kΩ resistor. Its charging time depends on the capacitance.

**2. Threshold Detection Stage**

The LM358 monitors the sensor voltage and generates a switching signal when the voltage reaches the reference threshold.

**3. Transistor Interface Stage**

Two C1815 NPN transistors are used:

- Q1 interfaces the LM358 output with ESP32 GPIO27.
- Q2 controls the discharge of the sensor capacitor through GPIO26.

All circuit grounds must be connected together. The ESP32 GPIO pins must only receive signals within the 3.3 V logic range.

---

## 5. LCD Interface

A 16×2 I2C LCD is used to display the measurement results.

The LCD is connected through the ESP32 I2C interface:

| LCD Pin | ESP32 Pin |
|---|---|
| SDA | GPIO21 |
| SCL | GPIO22 |
| VCC | According to LCD module requirements |
| GND | GND |

The LCD I2C address used in the firmware is:

```cpp
LiquidCrystal_I2C lcd(0x27, 16, 2);
```

The display presents two parameters:

**First row:** Measured capacitance.

**Second row:** Estimated displacement.

Example:

```text
C:0.765nF
Dist:4.00mm
```

If the threshold is not detected within the measurement timeout, the LCD displays a timeout message.

---

## 6. Experimental Calibration

### 6.1 Why Calibration Is Necessary

The theoretical parallel-plate capacitor equation does not fully describe the behavior of the assembled experimental sensor.

Practical measurements can be affected by:

- Parasitic capacitance.
- PCB geometry and electrode alignment.
- Electrical wiring and connections.
- Environmental influences.
- Mechanical positioning and gap uncertainty.
- Limitations of the analog measurement interface.

For this reason, an experimental calibration model is used to convert measured capacitance into displacement.

### 6.2 PCHIP Piecewise Cubic Interpolation

The firmware implements a piecewise cubic polynomial function called `calculateDistance()`.

Its input is the measured capacitance in nF, and its output is the estimated distance in mm.

The function divides the capacitance range into several intervals. Each interval has its own cubic polynomial:

$$
d(x)=a_ix^3+b_ix^2+c_ix+e_i
$$

Where:

- $x$: Measured capacitance (nF).
- $d(x)$: Estimated displacement (mm).
- $a_i,b_i,c_i,e_i$: Calibration coefficients for the corresponding interval.

The firmware uses a local variable:

$$
u=x-x_i
$$

This allows the polynomial to be evaluated relative to the beginning of each interval.

The implementation follows a PCHIP-style piecewise cubic calibration approach, providing a smooth interpolation between calibration points while preserving the experimentally established relationship.

### 6.3 Calibration Range

The current firmware contains the following capacitance breakpoints:

| Capacitance (nF) | Estimated distance (mm) |
|---:|---:|
| 0.577 | 11 |
| 0.592 | 10 |
| 0.606 | 9 |
| 0.620 | 8 |
| 0.635 | 7 |
| 0.664 | 6 |
| 0.692 | 5 |
| 0.765 | 4 |
| 0.837 | 3 |
| 0.981 | 2 |

These values represent the calibration points encoded in the current firmware.

**Note:** The function also clamps measurements outside the calibrated range to the endpoint distances: 11 mm for capacitance at or below 0.577 nF, and 2 mm for capacitance at or above 0.981 nF. Such values should not be interpreted as reliable extrapolated measurements.

---

## 7. Firmware and Software

The firmware is developed using the Arduino framework for ESP32.

### 7.1 Libraries

```cpp
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
```

### 7.2 Measurement Parameters

```cpp
const float R_CHARGE = 100000.0;
const float THRESHOLD_FACTOR = 0.693147;

const unsigned long DISCHARGE_TIME_US = 1000;
const unsigned long TIMEOUT_US = 1000000;
const unsigned long CYCLE_INTERVAL_MS = 500;
```

| Parameter | Value | Purpose |
|---|---:|---|
| Charging resistance | 100 kΩ | Controls capacitor charging |
| Threshold factor | 0.693147 | Corresponds to ln(2) |
| Discharge duration | 1000 µs | Resets sensor capacitor |
| Measurement timeout | 1,000,000 µs | Maximum threshold waiting time |
| Measurement interval | 500 ms | Delay between measurement cycles |
| Serial baud rate | 115200 | Serial communication |

### 7.3 Capacitance Calculation

The elapsed time is converted into seconds:

$$
t_s=t_{\mu s}\times10^{-6}
$$

The capacitance in farads is calculated as:

$$
C_F=\frac{t_s}{0.693147R}
$$

The result is converted into nanofarads:

$$
C_{nF}=C_F\times10^9
$$

### 7.4 Distance Calculation

The measured capacitance is passed to:

```cpp
float distance_mm = calculateDistance(capacitance_nF);
```

The function evaluates the appropriate polynomial interval and returns the estimated displacement in millimeters.

### 7.5 Output

The Serial Monitor displays:

```text
C = 0.765 nF | t = ... us | Distance = 4.00 mm
```

The LCD displays the capacitance and estimated distance.

The firmware also handles timeout conditions when no threshold transition is detected.

---

## 8. Experimental Characterization

The experimental evaluation focuses on the relationship between electrode displacement and measured capacitance.

The following tests can be used to characterize the system:

- Capacitance versus displacement.
- RC charging time versus displacement.
- Electronic circuit measurements versus multimeter measurements.
- Influence of charging resistance on measurement behavior.
- Repeatability at fixed displacement positions.
- Calibration error and measurement resolution.

### Recommended Graphs

| Graph | Purpose |
|---|---|
| Capacitance vs. displacement | Characterize the sensing element |
| Charging time vs. displacement | Evaluate the RC interface |
| Circuit vs. multimeter | Compare measurement methods |
| Resistance comparison | Study the influence of charging resistance |
| Calibration curve | Show the capacitance-to-distance relationship |
| Measurement error | Evaluate the accuracy of the system |

---

## 9. Repository Structure

```text
capacitive-displacement-sensor/
│
├── README.md
│
├── docs/
│   ├── report/
│   │   └── project-report.pdf
│   │
│   ├── images/
│   │   ├── project-banner.png
│   │   ├── sensor-electrodes.jpg
│   │   ├── system-block-diagram.png
│   │   ├── electronic-circuit.png
│   │   ├── lcd-display.jpg
│   │   ├── capacitance-comparison.png
│   │   ├── resistance-comparison.png
│   │   ├── calibration-curve.png
│   │   └── serial-monitor.png
│   │
│   └── schematics/
│       ├── circuit-schematic.pdf
│       └── circuit-schematic.png
│
├── firmware/
│   ├── capacitive_sensor/
│   │   └── capacitive_sensor.ino
│   │
│   └── calibration/
│       └── calibration.ino
│
├── experiments/
│   ├── raw-data/
│   │   ├── rc_100k.csv
│   │   ├── rc_1M.csv
│   │   ├── rc_4.7M.csv
│   │   └── multimeter_reference.csv
│   │
│   ├── graphs/
│   │   ├── capacitance_vs_displacement.png
│   │   ├── resistance_comparison.png
│   │   └── measurement_error.png
│   │
│   └── analysis/
│       └── experimental_analysis.ipynb
│
└── hardware/
    ├── bill-of-materials.md
    ├── pcb-design/
    └── mechanical-design/
```

---

## 10. Limitations and Future Improvements

### Current Limitations

- The measurement depends on the quality of the experimental calibration.
- Parasitic capacitance can influence the measured capacitance.
- The LM358 is an operational amplifier rather than a dedicated high-speed comparator.
- The current firmware uses a fixed charging resistance.
- Measurements outside the calibrated capacitance range are clamped to endpoint distances.
- Mechanical alignment and stability influence the measurement results.

### Future Improvements

- Improve the analog threshold detection stage.
- Investigate more stable capacitance measurement techniques.
- Reduce parasitic capacitance through improved PCB layout and shielding.
- Increase calibration data density.
- Evaluate measurement repeatability and uncertainty.
- Implement digital filtering or averaging.
- Compare different RC charging resistances.
- Investigate frequency-based or AC excitation measurement methods.

---

## 11. Tools and Technologies

- **Microcontroller:** ESP32
- **Analog electronics:** LM358, C1815 transistors, RC circuit
- **Sensing technology:** Parallel-plate capacitive sensing
- **Display:** 16×2 I2C LCD
- **Programming:** C/C++ with Arduino framework
- **Data analysis:** Python
- **Electronic design:** Proteus
- **Mechanical design:** Autodesk Fusion 360

---

## 12. Author

**Abderraouf Brada**

Electronics and Sensors Engineering Student

- GitHub: [abderraoufbrada](https://github.com/abderraoufbrada)
- LinkedIn: [Abderraouf Brada](https://www.linkedin.com/in/abderraouf-brada-16bab632a/)

---

## 13. Project Status

**Experimental prototype – Under development**

The prototype demonstrates the conversion of capacitive variations into an estimated displacement using an RC timing interface, an ESP32, and an LCD display.

Further experimental validation and improvements to the electronic measurement stage are required to establish the final measurement accuracy and repeatability.
