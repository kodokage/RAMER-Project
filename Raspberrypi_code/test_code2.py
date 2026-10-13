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
- Displays the resulting scan data.
- Ensures the motors are stopped and the controller is disconnected
  safely when the program finishes or an error occurs.

The robot is controlled using the `RobotController` class from the
`ramer` package.
"""



from ramer import RobotController
import sys
import select
import time


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

    # -----------------------------------------------
    # Start motors
    # -----------------------------------------------

    print("")
    print("Starting motors...")
    print("Press ENTER to stop.")
    print("")

    # Forward: direction 1
    robot.set_motors(150, 1, 150, 1)

    # -----------------------------------------------
    # Monitor encoders while moving
    # -----------------------------------------------

    while True:

        # Read encoder values
        encoders = robot.read_encoders()

        print(
            "Encoders: right={0} left={1}".format(
                encoders["right"],
                encoders["left"]
            )
        )

        # Check whether ENTER was pressed
        ready, _, _ = select.select(
            [sys.stdin],
            [],
            [],
            0.1
        )

        if ready:
            sys.stdin.readline()
            break

    # -----------------------------------------------
    # Stop motors
    # -----------------------------------------------

    print("Stopping motors...")
    robot.stop()

    # -----------------------------------------------
    # 180-degree yaw scan
    # -----------------------------------------------

    print("")
    print("Starting 180-degree yaw scan...")

    # start_pitch=end_pitch=0 means do not move pitch.
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