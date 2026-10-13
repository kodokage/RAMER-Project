# R.AM.E.R. Python API

Python API for controlling the R.AM.E.R. robot from a Raspberry Pi using an Arduino Mega over USB serial.

The API communicates with the Arduino using a simple text-based protocol at **9600 baud** and does **not require PySerial**.

## Requirements

- Raspberry Pi
- Python 3.2 or newer
- Arduino Mega
- R.AM.E.R. Arduino firmware
- USB connection between Raspberry Pi and Arduino
- Serial communication at 9600 baud

Default serial port:

```text
/dev/ttyUSB0
```

## Project Structure

```text
your_project/
├── example.py
└── ramer/
    ├── __init__.py
    └── robot_controller.py
```

Import the API with:

```python
from ramer import RobotController
```

No external Python packages are required.

---

# Getting Started

Create a controller:

```python
from ramer import RobotController

robot = RobotController("/dev/ttyUSB0")
```

Connect to the Arduino:

```python
robot.connect()
```

Read a sensor:

```python
print(robot.read_line_sensors())
```

Disconnect when finished:

```python
robot.disconnect()
```

For robots with motors, always make sure the motors are stopped when the program exits:

```python
try:
    robot.connect()

    # Robot commands here

finally:
    try:
        robot.stop()
    except Exception:
        pass

    robot.disconnect()
```

---

# RobotController

```python
RobotController(port="/dev/ttyUSB0", timeout=2.0)
```

## Parameters

| Parameter | Default | Description |
|---|---:|---|
| `port` | `/dev/ttyUSB0` | Arduino serial device |
| `timeout` | `2.0` | Serial response timeout in seconds |

## Connect

```python
robot.connect()
```

Opens the serial connection and configures it for **9600 baud**.

The Arduino is given approximately two seconds to reset after the serial connection is opened.

## Disconnect

```python
robot.disconnect()
```

Closes the serial connection.

---

# Motors

## `set_motors()`

Controls both drive motors.

```python
robot.set_motors(
    left_speed,
    left_direction,
    right_speed,
    right_direction
)
```

### Speed

Motor speed is an integer from:

```text
0 - 255
```

### Direction

```text
1 = Forward
0 = Reverse
```

### Example

```python
robot.set_motors(150, 1, 150, 1)
```

This drives both motors forward at speed `150`.

The Arduino firmware applies the configured right motor compensation automatically.

## `stop()`

Stops both motors:

```python
robot.stop()
```

---

# Line Sensors

## `read_line_sensors()`

Reads the three QRE1113 line sensors.

```python
line = robot.read_line_sensors()

print(line)
```

Returns:

```python
{
    "qre1": 67,
    "qre2": 66,
    "qre3": 64
}
```

The values are the raw Arduino `analogRead()` values.

---

# OPB706B Sensors

## `read_opto_sensors()`

Reads the three OPB706B sensors.

```python
opto = robot.read_opto_sensors()

print(opto)
```

Returns:

```python
{
    "opb1": 61,
    "opb2": 1008,
    "opb3": 1023
}
```

Values are raw ADC readings from `0` to `1023`.

---

# Distance Sensor

## `read_distance()`

Reads the Sharp distance sensor.

```python
distance = robot.read_distance()

print(distance)
```

Returns:

```python
{
    "yaw": 90,
    "pitch": 90,
    "adc": 650,
    "distance_cm": 14.96
}
```

| Value | Description |
|---|---|
| `yaw` | Current yaw servo angle |
| `pitch` | Current pitch servo angle |
| `adc` | Raw ADC reading |
| `distance_cm` | Calculated distance in centimetres |

---

# Wheel Encoders

## `read_encoders()`

Reads the current wheel encoder counts.

```python
encoders = robot.read_encoders()

print(encoders)
```

Returns:

```python
{
    "right": 1234,
    "left": 1201
}
```

The encoder values are cumulative counts from the Arduino.

They can be read while the motors are running:

```python
robot.set_motors(150, 1, 150, 1)

while True:
    encoders = robot.read_encoders()

    print(
        "Right:",
        encoders["right"],
        "Left:",
        encoders["left"]
    )
```

---

# Accelerometer

## `read_accelerometer()`

Reads the ADXL345 accelerometer.

```python
accel = robot.read_accelerometer()

print(accel)
```

Returns:

```python
{
    "x": -4,
    "y": -38,
    "z": -281
}
```

---

# Gyroscope

## `read_gyroscope()`

Reads the ITG3200 gyroscope.

```python
gyro = robot.read_gyroscope()

print(gyro)
```

Returns:

```python
{
    "x": 252,
    "y": 96,
    "z": -70
}
```

---

# Magnetometer

## `read_magnetometer()`

Reads the HMC5883L magnetometer.

```python
mag = robot.read_magnetometer()

print(mag)
```

Returns:

```python
{
    "x": -317,
    "y": -118,
    "z": 241
}
```

---

# Orientation

## `read_orientation()`

Calculates the robot's roll, pitch and heading using the accelerometer and magnetometer.

```python
orientation = robot.read_orientation()

print(orientation)
```

Returns:

```python
{
    "roll": -174.43,
    "pitch": 0.41,
    "heading": 156.13
}
```

All angles are returned in degrees.

| Value | Description |
|---|---|
| `roll` | Roll angle |
| `pitch` | Pitch angle |
| `heading` | Magnetic heading |

---

# Servos

## `set_servo()`

Controls the yaw and pitch servos.

```python
robot.set_servo(yaw, pitch)
```

Servo angles are:

```text
0 - 180 degrees
```

Example:

```python
robot.set_servo(90, 90)
```

Move yaw to `45°` and pitch to `90°`:

```python
robot.set_servo(45, 90)
```

The Arduino waits briefly after moving the servos before taking measurements.

---

# Scanning

## `scan()`

Performs a distance scan by moving the yaw and optionally the pitch servo.

```python
results = robot.scan(
    start_yaw,
    end_yaw,
    start_pitch,
    end_pitch,
    num_scans
)
```

### Example

Perform 50 measurements while moving the yaw servo from `1°` to `179°`:

```python
scan_data = robot.scan(
    1,
    179,
    0,
    0,
    50
)
```

When:

```python
start_pitch = 0
end_pitch = 0
```

the Arduino does not move the pitch servo.

### Processing Scan Results

Each scan point contains:

```python
{
    "yaw": 90,
    "pitch": 90,
    "adc": 650,
    "distance_cm": 14.96
}
```

Example:

```python
for point in scan_data:
    print(
        "Yaw:",
        point["yaw"],
        "Distance:",
        point["distance_cm"],
        "cm"
    )
```

---

# Complete Example

```python
from ramer import RobotController
import sys
import select


robot = RobotController("/dev/ttyUSB0")

try:
    robot.connect()

    print("Line:", robot.read_line_sensors())
    print("OPTO:", robot.read_opto_sensors())
    print("Distance:", robot.read_distance())
    print("Encoders:", robot.read_encoders())
    print("Accelerometer:", robot.read_accelerometer())
    print("Gyroscope:", robot.read_gyroscope())
    print("Magnetometer:", robot.read_magnetometer())
    print("Orientation:", robot.read_orientation())

    print()
    print("Starting motors...")
    print("Press ENTER to stop.")

    robot.set_motors(150, 1, 150, 1)

    while True:
        encoders = robot.read_encoders()

        print(
            "Right: {0}  Left: {1}".format(
                encoders["right"],
                encoders["left"]
            )
        )

        ready, _, _ = select.select(
            [sys.stdin],
            [],
            [],
            0.1
        )

        if ready:
            sys.stdin.readline()
            break

    robot.stop()

    print("Starting scan...")

    scan_data = robot.scan(
        1,
        179,
        0,
        0,
        50
    )

    for point in scan_data:
        print(point)

finally:
    try:
        robot.stop()
    except Exception:
        pass

    robot.disconnect()
```

---

# API Reference

| Function | Description |
|---|---|
| `connect()` | Connect to the Arduino |
| `disconnect()` | Disconnect from the Arduino |
| `set_motors()` | Set left and right motor speed/direction |
| `stop()` | Stop both motors |
| `read_line_sensors()` | Read the three QRE sensors |
| `read_opto_sensors()` | Read the three OPB706B sensors |
| `read_distance()` | Read the Sharp distance sensor |
| `read_encoders()` | Read wheel encoder counts |
| `read_accelerometer()` | Read ADXL345 |
| `read_gyroscope()` | Read ITG3200 |
| `read_magnetometer()` | Read HMC5883L |
| `read_orientation()` | Calculate roll, pitch and heading |
| `set_servo()` | Set yaw and pitch servo positions |
| `scan()` | Perform a servo distance scan |

---

# Serial Protocol

The Raspberry Pi communicates with the Arduino using ASCII text commands.

Communication speed:

```text
9600 baud
```

Each command ends with a newline.

## Motor

Pi sends:

```text
M 150 1 150 1
```

Arduino responds:

```text
OK MOTOR,150,1,200,1
```

## Stop

Pi sends:

```text
STOP
```

Arduino responds:

```text
OK STOP
```

## QRE Sensors

Pi sends:

```text
READ QRE
```

Arduino responds:

```text
QRE,67,66,64
```

## OPB Sensors

Pi sends:

```text
READ OPTO
```

Arduino responds:

```text
OPTO,61,1008,1023
```

## Distance

Pi sends:

```text
READ DISTANCE
```

Arduino responds:

```text
DISTANCE,90,90,650,14.96
```

## Encoders

Pi sends:

```text
READ ENCODERS
```

Arduino responds:

```text
ENCODERS,1234,1201
```

## Accelerometer

Pi sends:

```text
READ ACCEL
```

Arduino responds:

```text
ACCEL,-4,-38,-281
```

## Gyroscope

Pi sends:

```text
READ GYRO
```

Arduino responds:

```text
GYRO,252,96,-70
```

## Magnetometer

Pi sends:

```text
READ MAG
```

Arduino responds:

```text
MAG,-317,-118,241
```

## Orientation

Pi sends:

```text
READ ORIENTATION
```

Arduino responds:

```text
ORIENTATION,-174.43,0.41,156.13
```

## Servo

Pi sends:

```text
SERVO 90 90
```

Arduino responds:

```text
OK SERVO,90,90
```

## Scan

Pi sends:

```text
SCAN 1 179 0 0 50
```

Arduino responds with:

```text
SCAN_BEGIN,50
SCAN,1,90,650,14.96
SCAN,5,90,640,15.21
...
SCAN_END
```

---

# Error Handling

The API can raise exceptions if communication fails or the Arduino returns an error.

## `RuntimeError`

Used for connection errors and Arduino `ERR` responses.

Example:

```python
try:
    robot.set_motors(150, 1, 150, 1)

except RuntimeError as error:
    print("Robot error:", error)
```

## `ValueError`

Used when an invalid value or malformed Arduino response is received.

Example:

```python
try:
    data = robot.read_distance()

except ValueError as error:
    print("Invalid sensor data:", error)
```

## `TimeoutError`

Raised when the Arduino does not respond within the configured timeout.

Example:

```python
try:
    data = robot.read_distance()

except TimeoutError:
    print("Arduino did not respond")
```

---

# Troubleshooting

## Arduino does not respond

Check that the Arduino is connected:

```bash
ls /dev/ttyUSB*
```

You should normally see:

```text
/dev/ttyUSB0
```

If your device is different, use it when creating the controller:

```python
robot = RobotController("/dev/ttyUSB1")
```

## Permission denied

Check the serial device permissions:

```bash
ls -l /dev/ttyUSB0
```

Your Raspberry Pi user may need to be a member of the `dialout` group.

## No sensor response

Make sure:

- The Arduino firmware is uploaded.
- The Arduino is connected.
- The baud rate is `9600`.
- The correct serial device is being used.
- The Arduino has finished resetting after connection.

## Python version

The API is designed to support older Raspberry Pi systems, including **Python 3.2**.

Do not use APIs unavailable in Python 3.2.

---

# Safety

When controlling a physical robot, always provide a way to stop the motors.

Use a `finally` block:

```python
finally:
    try:
        robot.stop()
    except Exception:
        pass

    robot.disconnect()
```

This ensures that the program attempts to stop the robot even if an exception occurs.

