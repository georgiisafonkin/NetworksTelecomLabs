import time
from collections import deque


class InstantaneousSpeedCalculator:
    def __init__(self, time_interval):
        self.time_interval = time_interval
        self.arrival_times = deque()  # To store (time, size) tuples

    def add_chunk(self, chunk_size):
        current_time = time.time()
        self.arrival_times.append((current_time, chunk_size))
        # Remove chunks that are older than the time interval
        while self.arrival_times and (current_time - self.arrival_times[0][0] > self.time_interval):
            self.arrival_times.popleft()


    def calculate_instantaneous_speed(self):
        total_data = sum(size for _, size in self.arrival_times)
        return total_data  # Return total data in the last time interval
