"""
Robot sensor, motor control, encoder monitoring, and yaw scanning script.

This script connects to a RAMER robot through the serial interface
/dev/ttyUSB0 and demonstrates basic robot control and sensor feedback.

The program:
- Connects to the robot controller.
- Reads and displays line, opto, distance, encoder, accelerometer,
  gyroscope, magnetometer, and orientation data.
- Drives both motors forward at a specified speed.
- Continuously monitors and displays the left and right encoder values.
- Stops the robot when the user presses ENTER.
- Performs a 180-degree yaw scan while keeping the pitch fixed at zero.
- Ensures the motors are stopped and the controller is disconnected
  safely when the program finishes or an error occurs.

The robot is controlled using the `RobotController` class from the
`ramer` package.
"""



from ramer import RobotController

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

    # Forward: direction 1
    robot.set_motors(150, 1, 150, 1)

    input("Press ENTER to stop motors...")

    robot.stop()

    # 180-degree yaw scan at the current pitch.
    # start_pitch=end_pitch=0 means do not move pitch.
    scan_data = robot.scan(1, 179, 0, 0, 50)

    for point in scan_data:
        print(point)

finally:
    try:
        robot.stop()
    except Exception:
        pass

    robot.disconnect()
