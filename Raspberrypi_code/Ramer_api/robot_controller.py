import os
import termios
import select
import time


class RobotTimeoutError(Exception):
    """Raised when the Arduino does not respond in time."""
    pass


class RobotController:
    """
    R.AM.E.R. Raspberry Pi API.

    Compatible with Python 3.2.

    Communicates with the Arduino Mega over:
        /dev/ttyUSB0
        9600 baud

    No pyserial required.
    """

    def __init__(self, port="/dev/ttyUSB0", timeout=2.0):
        self.port = port
        self.timeout = timeout
        self.fd = None
        self.buffer = b""

    # =================================================
    # CONNECTION
    # =================================================

    def connect(self):
        if self.fd is not None:
            return

        print("Connecting to " + self.port)

        self.fd = os.open(
            self.port,
            os.O_RDWR |
            os.O_NOCTTY |
            os.O_NONBLOCK
        )

        attrs = termios.tcgetattr(self.fd)

        # Raw input/output
        attrs[0] = 0
        attrs[1] = 0
        attrs[2] = (
            termios.CS8 |
            termios.CREAD |
            termios.CLOCAL
        )
        attrs[3] = 0

        # 9600 baud
        attrs[4] = termios.B9600
        attrs[5] = termios.B9600

        termios.tcsetattr(
            self.fd,
            termios.TCSANOW,
            attrs
        )

        # Arduino Mega may reset when USB serial opens.
        time.sleep(2.0)

        # Remove startup text such as RAMER_READY.
        self._clear_input()

        print("Serial communication ready")

    def disconnect(self):
        if self.fd is not None:
            os.close(self.fd)

        self.fd = None
        self.buffer = b""

    # =================================================
    # TIME
    # =================================================

    def _now(self):
        """
        Python 3.2 does not have time.monotonic().
        time.time() is used for compatibility.
        """
        return time.time()

    # =================================================
    # CONNECTION CHECK
    # =================================================

    def _require_connection(self):
        if self.fd is None:
            raise RuntimeError(
                "RobotController is not connected"
            )

    # =================================================
    # SERIAL INPUT
    # =================================================

    def _clear_input(self):
        self._require_connection()

        while True:
            ready, _, _ = select.select(
                [self.fd],
                [],
                [],
                0
            )

            if not ready:
                break

            try:
                data = os.read(
                    self.fd,
                    256
                )

                if not data:
                    break

            except OSError:
                break

        self.buffer = b""

    # =================================================
    # SERIAL OUTPUT
    # =================================================

    def _send(self, command):
        self._require_connection()

        message = (
            command + "\n"
        ).encode("ascii")

        total = 0

        while total < len(message):

            try:
                written = os.write(
                    self.fd,
                    message[total:]
                )

                if written > 0:
                    total += written

            except OSError:
                # Wait until serial port is writable.
                select.select(
                    [],
                    [self.fd],
                    [],
                    1.0
                )

    # =================================================
    # READ LINE
    # =================================================

    def _readline(self, timeout=None):
        self._require_connection()

        if timeout is None:
            timeout = self.timeout

        deadline = (
            self._now() + timeout
        )

        while True:

            # -----------------------------------------
            # Check buffered data first
            # -----------------------------------------

            newline = self.buffer.find(b"\n")

            if newline >= 0:

                line = self.buffer[:newline]

                self.buffer = (
                    self.buffer[newline + 1:]
                )

                return line.decode(
                    "ascii",
                    "replace"
                ).strip()

            # -----------------------------------------
            # Calculate remaining time
            # -----------------------------------------

            remaining = (
                deadline - self._now()
            )

            if remaining <= 0:
                raise RobotTimeoutError(
                    "Timed out waiting for Arduino response"
                )

            # -----------------------------------------
            # Wait for serial data
            # -----------------------------------------

            wait_time = min(
                0.1,
                remaining
            )

            ready, _, _ = select.select(
                [self.fd],
                [],
                [],
                wait_time
            )

            if not ready:
                continue

            # -----------------------------------------
            # Read available bytes
            # -----------------------------------------

            try:
                data = os.read(
                    self.fd,
                    256
                )

                if data:
                    self.buffer += data

            except OSError:
                continue

    # =================================================
    # REQUEST / RESPONSE
    # =================================================

    def _request(
        self,
        command,
        prefix,
        timeout=None
    ):

        if timeout is None:
            timeout = self.timeout

        self._send(command)

        deadline = (
            self._now() + timeout
        )

        while True:

            remaining = (
                deadline - self._now()
            )

            if remaining <= 0:
                raise RobotTimeoutError(
                    "Timed out waiting for " +
                    prefix
                )

            line = self._readline(
                remaining
            )

            if line.startswith(prefix):
                return line

            if line.startswith("ERR"):
                raise RuntimeError(line)

    # =================================================
    # MOTORS
    # =================================================

    def set_motors(
        self,
        left_speed,
        left_direction,
        right_speed,
        right_direction
    ):

        if not 0 <= left_speed <= 255:
            raise ValueError(
                "left_speed must be 0-255"
            )

        if not 0 <= right_speed <= 255:
            raise ValueError(
                "right_speed must be 0-255"
            )

        if left_direction not in (0, 1):
            raise ValueError(
                "left_direction must be 0 or 1"
            )

        if right_direction not in (0, 1):
            raise ValueError(
                "right_direction must be 0 or 1"
            )

        command = (
            "M {} {} {} {}".format(
                int(left_speed),
                int(left_direction),
                int(right_speed),
                int(right_direction)
            )
        )

        return self._request(
            command,
            "OK MOTOR"
        )

    def stop(self):
        return self._request(
            "STOP",
            "OK STOP"
        )

    # =================================================
    # QRE1113 LINE SENSORS
    # =================================================

    def read_line_sensors(self):

        line = self._request(
            "READ QRE",
            "QRE"
        )

        fields = line.split(",")

        if len(fields) != 4:
            raise ValueError(
                "Invalid QRE response: " + line
            )

        return {
            "qre1": int(fields[1]),
            "qre2": int(fields[2]),
            "qre3": int(fields[3])
        }

    # =================================================
    # OPB706B
    # =================================================

    def read_opto_sensors(self):

        line = self._request(
            "READ OPTO",
            "OPTO"
        )

        fields = line.split(",")

        if len(fields) != 4:
            raise ValueError(
                "Invalid OPTO response: " + line
            )

        return {
            "opb1": int(fields[1]),
            "opb2": int(fields[2]),
            "opb3": int(fields[3])
        }

    # =================================================
    # SHARP DISTANCE
    # =================================================

    def read_distance(self):

        line = self._request(
            "READ DISTANCE",
            "DISTANCE"
        )

        fields = line.split(",")

        if len(fields) != 5:
            raise ValueError(
                "Invalid distance response: " + line
            )

        return {
            "yaw": int(fields[1]),
            "pitch": int(fields[2]),
            "adc": int(fields[3]),
            "distance_cm": float(fields[4])
        }

    # =================================================
    # ENCODERS
    # =================================================

    def read_encoders(self):

        line = self._request(
            "READ ENCODERS",
            "ENCODERS"
        )

        fields = line.split(",")

        if len(fields) != 3:
            raise ValueError(
                "Invalid encoder response: " + line
            )

        return {
            "right": int(fields[1]),
            "left": int(fields[2])
        }

    # =================================================
    # ACCELEROMETER
    # =================================================

    def read_accelerometer(self):

        line = self._request(
            "READ ACCEL",
            "ACCEL"
        )

        fields = line.split(",")

        if len(fields) != 4:
            raise ValueError(
                "Invalid accelerometer response: " + line
            )

        return {
            "x": int(fields[1]),
            "y": int(fields[2]),
            "z": int(fields[3])
        }

    # =================================================
    # GYROSCOPE
    # =================================================

    def read_gyroscope(self):

        line = self._request(
            "READ GYRO",
            "GYRO"
        )

        fields = line.split(",")

        if len(fields) != 4:
            raise ValueError(
                "Invalid gyroscope response: " + line
            )

        return {
            "x": int(fields[1]),
            "y": int(fields[2]),
            "z": int(fields[3])
        }

    # =================================================
    # MAGNETOMETER
    # =================================================

    def read_magnetometer(self):

        line = self._request(
            "READ MAG",
            "MAG"
        )

        fields = line.split(",")

        if len(fields) != 4:
            raise ValueError(
                "Invalid magnetometer response: " + line
            )

        return {
            "x": int(fields[1]),
            "y": int(fields[2]),
            "z": int(fields[3])
        }

    # =================================================
    # ORIENTATION
    # =================================================

    def read_orientation(self):

        line = self._request(
            "READ ORIENTATION",
            "ORIENTATION"
        )

        fields = line.split(",")

        if len(fields) != 4:
            raise ValueError(
                "Invalid orientation response: " + line
            )

        return {
            "roll": float(fields[1]),
            "pitch": float(fields[2]),
            "heading": float(fields[3])
        }

    # =================================================
    # SERVOS
    # =================================================

    def set_servo(self, yaw, pitch):

        if not 0 <= yaw <= 180:
            raise ValueError(
                "yaw must be between 0 and 180"
            )

        if not 0 <= pitch <= 180:
            raise ValueError(
                "pitch must be between 0 and 180"
            )

        command = (
            "SERVO {} {}".format(
                int(yaw),
                int(pitch)
            )
        )

        return self._request(
            command,
            "OK SERVO"
        )

    # =================================================
    # SCAN
    # =================================================

    def scan(
        self,
        start_yaw,
        end_yaw,
        start_pitch,
        end_pitch,
        num_scans
    ):

        num_scans = int(num_scans)

        if num_scans < 1:
            raise ValueError(
                "num_scans must be at least 1"
            )

        command = (
            "SCAN {} {} {} {} {}".format(
                int(start_yaw),
                int(end_yaw),
                int(start_pitch),
                int(end_pitch),
                num_scans
            )
        )

        self._send(command)

        results = []

        # Arduino moves the servo and waits 250 ms
        # for every scan point.
        deadline = (
            self._now()
            + self.timeout
            + (num_scans * 1.0)
        )

        while True:

            remaining = (
                deadline - self._now()
            )

            if remaining <= 0:
                raise RobotTimeoutError(
                    "Timed out waiting for scan"
                )

            line = self._readline(
                remaining
            )

            if line == "SCAN_END":
                return results

            if line.startswith("ERR"):
                raise RuntimeError(line)

            if line.startswith("SCAN,"):

                fields = line.split(",")

                if len(fields) != 5:
                    raise ValueError(
                        "Invalid scan response: " +
                        line
                    )

                results.append({
                    "yaw": int(fields[1]),
                    "pitch": int(fields[2]),
                    "adc": int(fields[3]),
                    "distance_cm": float(fields[4])
                })