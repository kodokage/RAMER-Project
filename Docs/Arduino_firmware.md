# R.AM.E.R. Arduino Firmware

Arduino Mega firmware for the **R.AM.E.R. robot**.

The Arduino acts as the robot's hardware controller. It receives commands from a Raspberry Pi over USB serial, controls the motors and servos, and returns sensor data.

Communication uses a simple text-based protocol at **9600 baud**.

---

# Hardware

The firmware is designed for an **Arduino Mega**.

## Pin Assignments

### QRE1113 Line Sensors

| Sensor | Arduino Pin |
|---|---|
| QRE 1 | `A0` |
| QRE 2 | `A2` |
| QRE 3 | `A4` |

### OPB706B Sensors

| Sensor | Arduino Pin |
|---|---|
| OPB 1 | `A6` |
| OPB 2 | `A8` |
| OPB 3 | `A10` |

### Sharp Distance Sensor

| Sensor | Arduino Pin |
|---|---|
| Sharp IR | `A13` |

### Servos

| Servo | Arduino Pin |
|---|---|
| Pitch | `7` |
| Yaw | `8` |

### Motors

| Motor | Direction | Speed |
|---|---:|---:|
| Right | `26` | `4` |
| Left | `24` | `5` |

### Encoders

| Encoder | Arduino Pin |
|---|---:|
| Right encoder | `2` |
| Left encoder | `3` |

---

# Serial Communication

The Arduino communicates with the Raspberry Pi using:

```text
9600 baud
