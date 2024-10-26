import time


class AverageSpeedCalculator:
    def __init__(self):
        self.chunk_sizes = []
        self.arrival_times = []
        self.start_time = time.time()


    def add_chunk(self, chunk_size):
        current_time = time.time()
        self.chunk_sizes.append(chunk_size)
        self.arrival_times.append(current_time)


    def calculate_average_speed(self):
        total_data_transferred = sum(self.chunk_sizes)
        total_time_taken = self.arrival_times[-1] - self.start_time
        return total_data_transferred / total_time_taken