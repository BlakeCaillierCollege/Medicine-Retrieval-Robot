"""Plot PID telemetry streamed by the robot's /get/PID endpoint."""

import struct
import sys

import requests

from graph import Graph


ESP_BASE_URL = "http://192.168.1.177"
SAMPLE_FORMAT = "<Ihhhh"
SAMPLE_SIZE = struct.calcsize(SAMPLE_FORMAT)
TICKS_PER_SECOND = 100  # CONFIG_FREERTOS_HZ in sdkconfig.esp32dev
UPDATE_MS = 250
WINDOW_SECONDS = 30

session = requests.Session()
graph = Graph("Live PID Data", window_seconds=WINDOW_SECONDS, ylabel="Speed (m/s)")
graph.add_line("Motor 1 target", color="#a855f7")
graph.add_line("Motor 1 measured", color="#22c55e")
graph.add_line("Motor 2 target", color="#f59e0b")
graph.add_line("Motor 2 measured", color="#38bdf8")

last_tick = None
elapsed_ticks = 0


def poll_pid():
    global last_tick, elapsed_ticks
    try:
        response = session.get(f"{ESP_BASE_URL}/get/PID", timeout=1)
        response.raise_for_status()
        payload = response.content
        if len(payload) % SAMPLE_SIZE:
            raise ValueError(f"Incomplete PID sample: {len(payload)} bytes")

        for tick, target1, measured1, target2, measured2 in struct.iter_unpack(
            SAMPLE_FORMAT, payload
        ):
            if last_tick is not None:
                elapsed_ticks += (tick - last_tick) & 0xFFFFFFFF
            last_tick = tick
            graph.add_sample(
                elapsed_ticks / TICKS_PER_SECOND,
                {
                    "Motor 1 target": target1 / 1000,
                    "Motor 1 measured": measured1 / 1000,
                    "Motor 2 target": target2 / 1000,
                    "Motor 2 measured": measured2 / 1000,
                },
            )
    except (requests.RequestException, ValueError) as exc:
        print(f"PID telemetry: {exc}", file=sys.stderr)


if __name__ == "__main__":
    graph.show(poll_pid, interval_ms=UPDATE_MS)
