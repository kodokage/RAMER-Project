/*
  Arduino Mega Robot Hardware Tester
  ----------------------------------
  Set ONE test section to 1 and all others to 0.

  Added:
  - 9DOF IMU (ADXL345 + ITG3200 + HMC5883L)

  Baud Rate: 9600
*/

#include <Servo.h>
#include <Wire.h>
#include <math.h>

//====================================================
// ENABLE ONE TEST AT A TIME
//====================================================

#define TEST_QRE1113_BOTTOM_IR      0    //Done
#define TEST_OPB706B_IR             0    //Done
#define TEST_SHARP_DISTANCE         0    //Done
#define TEST_SERVOS                 0    //Done
#define TEST_ENCODERS               1    //Done
#define TEST_MOTORS                 1    //Done

// ⭐ NEW: 9DOF IMU TEST
#define TEST_9DOF_IMU               0    //Done

//====================================================
// PIN ASSIGNMENTS
//====================================================

// QRE1113 Bottom IR Sensors
const int ir_sensor_1 = A0;
const int ir_sensor_2 = A2;
const int ir_sensor_3 = A4;

// OPB706B Sensors
const int ir_diode_transistor_1 = A6;
const int ir_diode_transistor_2 = A8;
const int ir_diode_transistor_3 = A10;

// Sharp Distance Sensor
const int ir_distance_sensor = A13;

// Servos
const int servo1_pitch = 7;
const int servo2_yaw = 8;

// Motor Outputs
const int right_motor_fwd = 26;
const int right_motor_speed = 4;

const int left_motor_fwd = 24;
const int left_motor_speed = 5;

// Encoders
const int right_wheel_encoder = 2;
const int left_wheel_encoder = 3;

//====================================================
// 9DOF SENSOR ADDRESSES (NEW SECTION)
//====================================================

#define ADXL345   0x53
#define ITG3200   0x68
#define HMC5883L  0x1E

//====================================================
// GLOBALS
//====================================================

Servo pitchServo;
Servo yawServo;

volatile long rightEncoderCount = 0;
volatile long leftEncoderCount = 0;

// 9DOF raw values
int16_t ax, ay, az;
int16_t gx, gy, gz;
int16_t mx, my, mz;

//====================================================
// INTERRUPTS
//====================================================

void rightEncoderISR() { rightEncoderCount++; }
void leftEncoderISR()  { leftEncoderCount++; }

//====================================================
// SETUP
//====================================================

void setup()
{
  Serial.begin(9600);
  delay(1000);

#if TEST_QRE1113_BOTTOM_IR
  pinMode(ir_sensor_1, INPUT);
  pinMode(ir_sensor_2, INPUT);
  pinMode(ir_sensor_3, INPUT);
  Serial.println("=== QRE1113 Bottom IR Test ===");
#endif

#if TEST_OPB706B_IR
  pinMode(ir_diode_transistor_1, INPUT);
  pinMode(ir_diode_transistor_2, INPUT);
  pinMode(ir_diode_transistor_3, INPUT);
  Serial.println("=== OPB706B Sensor Test ===");
#endif

#if TEST_SHARP_DISTANCE
  pinMode(ir_distance_sensor, INPUT);
  Serial.println("=== Sharp Distance Sensor Test ===");
#endif

#if TEST_SERVOS
  pitchServo.attach(servo1_pitch);
  yawServo.attach(servo2_yaw);
  Serial.println("=== Servo Test ===");
#endif

#if TEST_ENCODERS
  pinMode(right_wheel_encoder, INPUT_PULLUP);
  pinMode(left_wheel_encoder, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(right_wheel_encoder), rightEncoderISR, RISING);
  attachInterrupt(digitalPinToInterrupt(left_wheel_encoder), leftEncoderISR, RISING);

  Serial.println("=== Encoder Test ===");
#endif

#if TEST_MOTORS
  pinMode(right_motor_fwd, OUTPUT);
  pinMode(right_motor_speed, OUTPUT);
  pinMode(left_motor_fwd, OUTPUT);
  pinMode(left_motor_speed, OUTPUT);

  Serial.println("=== Motor Test ===");
#endif

//====================================================
// ⭐ 9DOF IMU INIT SECTION (NEW)
//====================================================

#if TEST_9DOF_IMU
  Wire.begin();

  initADXL345();
  initITG3200();
  initHMC5883L();

  Serial.println("=== 9DOF IMU TEST (ADXL345 + ITG3200 + HMC5883L) ===");
#endif
}

//====================================================
// LOOP
//====================================================

void loop()
{

#if TEST_QRE1113_BOTTOM_IR
  Serial.print("IR1="); Serial.print(analogRead(ir_sensor_1));
  Serial.print(" IR2="); Serial.print(analogRead(ir_sensor_2));
  Serial.print(" IR3="); Serial.println(analogRead(ir_sensor_3));
  delay(250);
#endif

#if TEST_OPB706B_IR
  Serial.print("OPB1="); Serial.print(analogRead(ir_diode_transistor_1));
  Serial.print(" OPB2="); Serial.print(analogRead(ir_diode_transistor_2));
  Serial.print(" OPB3="); Serial.println(analogRead(ir_diode_transistor_3));
  delay(250);
#endif

#if TEST_SHARP_DISTANCE
  int adc = analogRead(ir_distance_sensor);
  float distance_cm = 10650.08 * pow(adc, -0.935) - 10;

  Serial.print("ADC="); Serial.print(adc);
  Serial.print(" Distance="); Serial.println(distance_cm, 1);
  delay(250);
#endif

#if TEST_SERVOS
  static bool runOnce = false;
  if (!runOnce) {
    pitchServo.write(0); yawServo.write(0);
    delay(1000);
    pitchServo.write(90); yawServo.write(90);
    delay(1000);
    pitchServo.write(180); yawServo.write(180);
    delay(1000);
    runOnce = true;
  }
#endif

#if TEST_ENCODERS
  Serial.print("Right Enc="); Serial.print(rightEncoderCount);
  Serial.print(" Left Enc="); Serial.println(leftEncoderCount);
  delay(250);
#endif

#if TEST_MOTORS
  static bool motorInit = false;
  if (!motorInit) {
    digitalWrite(right_motor_fwd, HIGH);
    digitalWrite(left_motor_fwd, HIGH);
    analogWrite(right_motor_speed, 200);
    analogWrite(left_motor_speed, 150);
    Serial.println("Motors running");
    motorInit = true;
  }
  delay(250);
#endif

//====================================================
// ⭐ 9DOF IMU TEST LOOP (NEW SECTION)
//====================================================

#if TEST_9DOF_IMU

  readAccel();
  readGyro();
  readMag();

  float Ax = ax * 0.0039;
  float Ay = ay * 0.0039;
  float Az = az * 0.0039;

  float roll  = atan2(Ay, Az);
  float pitch = atan2(-Ax, sqrt(Ay * Ay + Az * Az));

  float Xh = mx * cos(pitch) + mz * sin(pitch);
  float Yh = mx * sin(roll) * sin(pitch)
           + my * cos(roll)
           - mz * sin(roll) * cos(pitch);

  float heading = atan2(Yh, Xh);
  heading = heading * 180.0 / PI;
  if (heading < 0) heading += 360;

  Serial.print("ACC:");
  Serial.print(ax); Serial.print(" ");
  Serial.print(ay); Serial.print(" ");
  Serial.print(az);

  Serial.print(" GYRO:");
  Serial.print(gx); Serial.print(" ");
  Serial.print(gy); Serial.print(" ");
  Serial.print(gz);

  Serial.print(" MAG:");
  Serial.print(mx); Serial.print(" ");
  Serial.print(my); Serial.print(" ");
  Serial.print(mz);

  Serial.print(" Roll:");
  Serial.print(roll * 180 / PI, 1);

  Serial.print(" Pitch:");
  Serial.print(pitch * 180 / PI, 1);

  Serial.print(" Heading:");
  Serial.println(heading, 1);

  delay(100);

#endif
}

//====================================================
// 9DOF FUNCTIONS
//====================================================

// ---- ADXL345 ----
void initADXL345()
{
  Wire.beginTransmission(ADXL345);
  Wire.write(0x2D);
  Wire.write(0x08);
  Wire.endTransmission();
}

void readAccel()
{
  Wire.beginTransmission(ADXL345);
  Wire.write(0x32);
  Wire.endTransmission(false);
  Wire.requestFrom(ADXL345, 6);

  ax = Wire.read() | (Wire.read() << 8);
  ay = Wire.read() | (Wire.read() << 8);
  az = Wire.read() | (Wire.read() << 8);
}

// ---- ITG3200 ----
void initITG3200()
{
  Wire.beginTransmission(ITG3200);
  Wire.write(0x3E);
  Wire.write(0x00);
  Wire.endTransmission();
}

void readGyro()
{
  Wire.beginTransmission(ITG3200);
  Wire.write(0x1D);
  Wire.endTransmission(false);
  Wire.requestFrom(ITG3200, 6);

  gx = (Wire.read() << 8) | Wire.read();
  gy = (Wire.read() << 8) | Wire.read();
  gz = (Wire.read() << 8) | Wire.read();
}

// ---- HMC5883L ----
void initHMC5883L()
{
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
}

void readMag()
{
  Wire.beginTransmission(HMC5883L);
  Wire.write(0x03);
  Wire.endTransmission(false);
  delay(6);
  Wire.requestFrom(HMC5883L, 6);

  mx = (Wire.read() << 8) | Wire.read();
  mz = (Wire.read() << 8) | Wire.read();
  my = (Wire.read() << 8) | Wire.read();
}
