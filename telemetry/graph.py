"""Reusable live graph with any number of time-based series."""

from collections import deque
from math import ceil

import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation


class Graph:
    def __init__(self, title, *, window_seconds=30, ylabel="Value", max_plot_points=600):
        if window_seconds <= 0:
            raise ValueError("window_seconds must be positive")
        if max_plot_points < 4:
            raise ValueError("max_plot_points must be at least 4")
        self.window_seconds = window_seconds
        self.max_plot_points = max_plot_points
        self.fig, self.ax = plt.subplots()
        self.fig.patch.set_facecolor("black")
        self.ax.set_facecolor("black")
        self.ax.set_title(title, color="white")
        self.ax.set_xlabel("Time (seconds)", color="white")
        self.ax.set_ylabel(ylabel, color="white")
        self.ax.tick_params(colors="white")
        for spine in self.ax.spines.values():
            spine.set_color("white")
        self.ax.set_xlim(0, window_seconds)
        self.series = {}
        self.latest_time = None
        self.animation = None

    def add_line(self, name, *, color):
        if name in self.series:
            raise ValueError(f"Line already exists: {name}")
        line, = self.ax.plot([], [], color=color, linewidth=2, label=name)
        self.series[name] = (line, deque())
        legend = self.ax.legend()
        for text in legend.get_texts():
            text.set_color("white")
        legend.get_frame().set_facecolor("#202020")

    def add_sample(self, time_seconds, values):
        if self.latest_time is not None and time_seconds < self.latest_time:
            raise ValueError("Samples must be added in time order")
        self.latest_time = time_seconds
        cutoff = time_seconds - self.window_seconds
        for name, value in values.items():
            if name not in self.series:
                raise KeyError(f"Unknown line: {name}")
            self.series[name][1].append((time_seconds, value))
        for _, points in self.series.values():
            while points and points[0][0] < cutoff:
                points.popleft()

    def refresh(self):
        for line, points in self.series.values():
            visible = list(points)
            if len(visible) > self.max_plot_points:
                # Keep the high and low value in each time bucket so spikes remain visible.
                bucket_size = ceil(len(visible) / ((self.max_plot_points - 2) // 2))
                reduced = [visible[0]]
                for start in range(0, len(visible), bucket_size):
                    bucket = visible[start : start + bucket_size]
                    low = min(bucket, key=lambda point: point[1])
                    high = max(bucket, key=lambda point: point[1])
                    reduced.extend(sorted((low, high), key=lambda point: point[0]))
                reduced.append(visible[-1])
                visible = reduced
            line.set_data([point[0] for point in visible], [point[1] for point in visible])
        if self.latest_time is not None:
            self.ax.set_xlim(
                max(0, self.latest_time - self.window_seconds),
                max(self.window_seconds, self.latest_time),
            )
        self.ax.relim()
        self.ax.autoscale_view(scalex=False, scaley=True)

    def show(self, poll, *, interval_ms=100):
        def update(_frame):
            poll()
            self.refresh()

        self.animation = FuncAnimation(
            self.fig, update, interval=interval_ms, cache_frame_data=False
        )
        plt.show()
