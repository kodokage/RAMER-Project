/*
  R.AM.E.R. Combined Arduino Firmware
  ------------------------------------
  Hardware <-> Arduino Mega <-> Raspberry Pi

  Serial: 9600 baud

  Motor command:
    M <leftSpeed> <leftDirection> <rightSpeed> <rightDirection>
    Direction: 1 = forward, 0 = reverse
    Right motor compensation: +50

  Sensor commands:
    READ QRE
    READ OPTO
    READ DISTANCE
    READ ENCODERS
    READ ACCEL
    READ GYRO
    READ MAG
    READ ORIENTATION

  Servo commands:
    SERVO <yaw> <pitch>

  Scan command:
    SCAN <startYaw> <endYaw> <startPitch> <endPitch> <numScans>

  Pitch rule:
    If startPitch == 0 AND endPitch == 0, the pitch servo is not moved.
*/

#include <Wire.h>
#include <Servo.h>
#include <math.h>

// ====================================================
// SERIAL
// ====================================================

const unsigned long SERIAL_BAUD = 9600;

// ====================================================
// PIN ASSIGNMENTS
// Source: supplied R.AM.E.R. I/O test implementation
// ====================================================

// QRE1113
const int ir_sensor_1 = A0;
const int ir_sensor_2 = A2;
const int ir_sensor_3 = A4;

// OPB706B
const int ir_diode_transistor_1 = A6;
const int ir_diode_transistor_2 = A8;
const int ir_diode_transistor_3 = A10;

// Sharp distance sensor
const int ir_distance_sensor = A13;

// Servos
const int servo1_pitch = 7;
const int servo2_yaw   = 8;

// Motors
const int right_motor_fwd   = 26;
const int right_motor_speed = 4;
const int left_motor_fwd    = 24;
const int left_motor_speed  = 5;

// Encoders
const int right_wheel_encoder = 2;
const int left_wheel_encoder  = 3;

// ====================================================
// MOTOR CALIBRATION
// ====================================================

const int RIGHT_MOTOR_OFFSET = 50;

// ====================================================
// IMU ADDRESSES
// ====================================================

#define ADXL345  0x53
#define ITG3200  0x68
#define HMC5883L 0x1E

// ====================================================
// GLOBALS
// ====================================================

Servo pitchServo;
Servo yawServo;

volatile long rightEncoderCount = 0;
volatile long leftEncoderCount  = 0;

int16_t ax, ay, az;
int16_t gx, gy, gz;
int16_t mx, my, mz;

int currentYaw = 90;
int currentPitch = 90;

// ====================================================
// ENCODERS
// ====================================================

void rightEncoderISR()
{
  rightEncoderCount++;
}

void leftEncoderISR()
{
  leftEncoderCount++;
}

// ====================================================
// SENSOR FUNCTIONS
// ====================================================

float sharpDistance(int adc)
{
  if (adc <= 0)
    return -1.0;

  return 10650.08 * pow((float)adc, -0.935) - 10.0;
}

void readAccel()
{
  Wire.beginTransmission(ADXL345);
  Wire.write(0x32);
  Wire.endTransmission(false);
  Wire.requestFrom(ADXL345, 6);

  if (Wire.available() >= 6)
  {
    ax = Wire.read() | (Wire.read() << 8);
    ay = Wire.read() | (Wire.read() << 8);
    az = Wire.read() | (Wire.read() << 8);
  }
}

void readGyro()
{
  Wire.beginTransmission(ITG3200);
  Wire.write(0x1D);
  Wire.endTransmission(false);
  Wire.requestFrom(ITG3200, 6);

  if (Wire.available() >= 6)
  {
    gx = (Wire.read() << 8) | Wire.read();
    gy = (Wire.read() << 8) | Wire.read();
    gz = (Wire.read() << 8) | Wire.read();
  }
}

void readMag()
{
  Wire.beginTransmission(HMC5883L);
  Wire.write(0x03);
  Wire.endTransmission(false);

  delay(6);

  Wire.requestFrom(HMC5883L, 6);

  if (Wire.available() >= 6)
  {
    mx = (Wire.read() << 8) | Wire.read();
    mz = (Wire.read() << 8) | Wire.read();
    my = (Wire.read() << 8) | Wire.read();
  }
}

// ====================================================
// MOTOR CONTROL
// ====================================================

void stopMotors()
{
  analogWrite(right_motor_speed, 0);
  analogWrite(left_motor_speed, 0);
}

void setMotor(int directionPin, int speedPin,
              int speed, int direction)
{
  speed = constrain(speed, 0, 255);
  digitalWrite(directionPin, direction ? HIGH : LOW);
  analogWrite(speedPin, speed);
}

void setWheels(int leftSpeed, int leftDirection,
               int rightSpeed, int rightDirection)
{
  if ((leftDirection != 0 && leftDirection != 1) ||
      (rightDirection != 0 && rightDirection != 1))
  {
    Serial.println("ERR INVALID_DIRECTION");
    return;
  }

  leftSpeed = constrain(leftSpeed, 0, 255);
  rightSpeed = constrain(rightSpeed + RIGHT_MOTOR_OFFSET, 0, 255);

  setMotor(left_motor_fwd, left_motor_speed,
           leftSpeed, leftDirection);

  setMotor(right_motor_fwd, right_motor_speed,
           rightSpeed, rightDirection);

  Serial.print("OK MOTOR,");
  Serial.print(leftSpeed);
  Serial.print(",");
  Serial.print(leftDirection);
  Serial.print(",");
  Serial.print(rightSpeed);
  Serial.print(",");
  Serial.println(rightDirection);
}

// ====================================================
// SERVO CONTROL
// ====================================================

void setServoAngles(int yaw, int pitch, bool movePitch)
{
  yaw = constrain(yaw, 0, 180);
  pitch = constrain(pitch, 0, 180);

  yawServo.write(yaw);
  currentYaw = yaw;

  if (movePitch)
  {
    pitchServo.write(pitch);
    currentPitch = pitch;
  }

  // Allow SG90 to move before measuring.
  delay(250);
}

// ====================================================
// INDIVIDUAL SENSOR RESPONSES
// ====================================================

void sendQRE()
{
  Serial.print("QRE,");
  Serial.print(analogRead(ir_sensor_1));
  Serial.print(",");
  Serial.print(analogRead(ir_sensor_2));
  Serial.print(",");
  Serial.println(analogRead(ir_sensor_3));
}

void sendOPTO()
{
  Serial.print("OPTO,");
  Serial.print(analogRead(ir_diode_transistor_1));
  Serial.print(",");
  Serial.print(analogRead(ir_diode_transistor_2));
  Serial.print(",");
  Serial.println(analogRead(ir_diode_transistor_3));
}

void sendDistance()
{
  int adc = analogRead(ir_distance_sensor);
  float distance = sharpDistance(adc);

  Serial.print("DISTANCE,");
  Serial.print(currentYaw);
  Serial.print(",");
  Serial.print(currentPitch);
  Serial.print(",");
  Serial.print(adc);
  Serial.print(",");
  Serial.println(distance, 2);
}

void sendEncoders()
{
  noInterrupts();
  long right = rightEncoderCount;
  long left = leftEncoderCount;
  interrupts();

  Serial.print("ENCODERS,");
  Serial.print(right);
  Serial.print(",");
  Serial.println(left);
}

void sendAccelerometer()
{
  readAccel();

  Serial.print("ACCEL,");
  Serial.print(ax);
  Serial.print(",");
  Serial.print(ay);
  Serial.print(",");
  Serial.println(az);
}

void sendGyroscope()
{
  readGyro();

  Serial.print("GYRO,");
  Serial.print(gx);
  Serial.print(",");
  Serial.print(gy);
  Serial.print(",");
  Serial.println(gz);
}

void sendMagnetometer()
{
  readMag();

  Serial.print("MAG,");
  Serial.print(mx);
  Serial.print(",");
  Serial.print(my);
  Serial.print(",");
  Serial.println(mz);
}

void sendOrientation()
{
  readAccel();
  readMag();

  float Ax = ax * 0.0039;
  float Ay = ay * 0.0039;
  float Az = az * 0.0039;

  float roll = atan2(Ay, Az);
  float pitch = atan2(-Ax, sqrt(Ay * Ay + Az * Az));

  float Xh = mx * cos(pitch) + mz * sin(pitch);

  float Yh =
    mx * sin(roll) * sin(pitch)
    + my * cos(roll)
    - mz * sin(roll) * cos(pitch);

  float heading = atan2(Yh, Xh) * 180.0 / PI;

  if (heading < 0)
    heading += 360.0;

  Serial.print("ORIENTATION,");
  Serial.print(roll * 180.0 / PI, 2);
  Serial.print(",");
  Serial.print(pitch * 180.0 / PI, 2);
  Serial.print(",");
  Serial.println(heading, 2);
}

// ====================================================
// SCAN
// ====================================================

void performScan(int startYaw, int endYaw,
                 int startPitch, int endPitch,
                 int numScans)
{
  if (numScans < 1)
  {
    Serial.println("ERR INVALID_SCAN_COUNT");
    return;
  }

  startYaw = constrain(startYaw, 0, 180);
  endYaw = constrain(endYaw, 0, 180);

  startPitch = constrain(startPitch, 0, 180);
  endPitch = constrain(endPitch, 0, 180);

  // Both zero means "leave pitch where it is".
  bool movePitch = !(startPitch == 0 && endPitch == 0);

  Serial.print("SCAN_BEGIN,");
  Serial.println(numScans);

  for (int i = 0; i < numScans; i++)
  {
    float fraction;

    if (numScans == 1)
      fraction = 0.0;
    else
      fraction = (float)i / (float)(numScans - 1);

    int yaw = startYaw +
              (int)round((endYaw - startYaw) * fraction);

    int pitch = startPitch +
                (int)round((endPitch - startPitch) * fraction);

    setServoAngles(yaw, pitch, movePitch);

    int adc = analogRead(ir_distance_sensor);
    float distance = sharpDistance(adc);

    Serial.print("SCAN,");
    Serial.print(currentYaw);
    Serial.print(",");
    Serial.print(currentPitch);
    Serial.print(",");
    Serial.print(adc);
    Serial.print(",");
    Serial.println(distance, 2);
  }

  Serial.println("SCAN_END");
}

// ====================================================
// COMMAND PROCESSING
// ====================================================

void processCommand(String command)
{
  command.trim();

  if (command.length() == 0)
    return;

  if (command == "STOP")
  {
    stopMotors();
    Serial.println("OK STOP");
    return;
  }

  if (command.startsWith("M "))
  {
    int leftSpeed, leftDirection;
    int rightSpeed, rightDirection;

    int n = sscanf(command.c_str(),
                   "M %d %d %d %d",
                   &leftSpeed, &leftDirection,
                   &rightSpeed, &rightDirection);

    if (n != 4)
    {
      Serial.println("ERR MOTOR_FORMAT");
      return;
    }

    setWheels(leftSpeed, leftDirection,
              rightSpeed, rightDirection);
    return;
  }

  if (command.startsWith("SERVO "))
  {
    int yaw, pitch;

    int n = sscanf(command.c_str(),
                   "SERVO %d %d",
                   &yaw, &pitch);

    if (n != 2)
    {
      Serial.println("ERR SERVO_FORMAT");
      return;
    }

    bool movePitch = !(pitch == 0);

    setServoAngles(yaw, pitch, movePitch);

    Serial.print("OK SERVO,");
    Serial.print(currentYaw);
    Serial.print(",");
    Serial.println(currentPitch);
    return;
  }

  if (command == "READ QRE")
  {
    sendQRE();
    return;
  }

  if (command == "READ OPTO")
  {
    sendOPTO();
    return;
  }

  if (command == "READ DISTANCE")
  {
    sendDistance();
    return;
  }

  if (command == "READ ENCODERS")
  {
    sendEncoders();
    return;
  }

  if (command == "READ ACCEL")
  {
    sendAccelerometer();
    return;
  }

  if (command == "READ GYRO")
  {
    sendGyroscope();
    return;
  }

  if (command == "READ MAG")
  {
    sendMagnetometer();
    return;
  }

  if (command == "READ ORIENTATION")
  {
    sendOrientation();
    return;
  }

  if (command.startsWith("SCAN "))
  {
    int startYaw, endYaw;
    int startPitch, endPitch;
    int numScans;

    int n = sscanf(command.c_str(),
                   "SCAN %d %d %d %d %d",
                   &startYaw, &endYaw,
                   &startPitch, &endPitch,
                   &numScans);

    if (n != 5)
    {
      Serial.println("ERR SCAN_FORMAT");
      return;
    }

    performScan(startYaw, endYaw,
                startPitch, endPitch,
                numScans);
    return;
  }

  Serial.println("ERR UNKNOWN_COMMAND");
}

// ====================================================
// SETUP
// ====================================================

void setup()
{
  Serial.begin(SERIAL_BAUD);

  // Sensor inputs
  pinMode(ir_sensor_1, INPUT);
  pinMode(ir_sensor_2, INPUT);
  pinMode(ir_sensor_3, INPUT);

  pinMode(ir_diode_transistor_1, INPUT);
  pinMode(ir_diode_transistor_2, INPUT);
  pinMode(ir_diode_transistor_3, INPUT);

  pinMode(ir_distance_sensor, INPUT);

  // Motors
  pinMode(right_motor_fwd, OUTPUT);
  pinMode(right_motor_speed, OUTPUT);
  pinMode(left_motor_fwd, OUTPUT);
  pinMode(left_motor_speed, OUTPUT);

  stopMotors();

  // Encoders
  pinMode(right_wheel_encoder, INPUT_PULLUP);
  pinMode(left_wheel_encoder, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(right_wheel_encoder),
                  rightEncoderISR, RISING);

  attachInterrupt(digitalPinToInterrupt(left_wheel_encoder),
                  leftEncoderISR, RISING);

  // Servos
  pitchServo.attach(servo1_pitch);
  yawServo.attach(servo2_yaw);

  yawServo.write(currentYaw);
  pitchServo.write(currentPitch);

  // IMU
  Wire.begin();

  // ADXL345
  Wire.beginTransmission(ADXL345);
  Wire.write(0x2D);
  Wire.write(0x08);
  Wire.endTransmission();

  // ITG3200
  Wire.beginTransmission(ITG3200);
  Wire.write(0x3E);
  Wire.write(0x00);
  Wire.endTransmission();

  // HMC5883L
  Wire.beginTransmission(HMC5883L);
  Wire.write(0x00);
  Wire.write(0x70);
  Wire.endTransmission();

  Wire.beginTransmission(HMC5883L);
  Wire.write(0x01);
  Wire.write(0x20);
  Wire.endTransmission();

  Wire.beginTransmission(HMC5883L);
  Wire.write(0x02);
  Wire.write(0x00);
  Wire.endTransmission();

  delay(500);

  Serial.println("RAMER_READY");
}

// ====================================================
// LOOP
// ====================================================

void loop()
{
  static String command = "";

  while (Serial.available() > 0)
  {
    char c = Serial.read();

    if (c == '\n' || c == '\r')
    {
      if (command.length() > 0)
      {
        processCommand(command);
        command = "";
      }
    }
    else
    {
      command += c;

      if (command.length() > 120)
      {
        command = "";
        Serial.println("ERR COMMAND_TOO_LONG");
      }
    }
  }
}
