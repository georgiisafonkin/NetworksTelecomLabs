import os
import sys
from math import ceil
from socket import *

CHUNK_DATA_SIZE = 1024
CHUNK_SIZE = 1024 + 512
def get_file_name(filepath):
    return filepath.split('/')[-1]


def read_single_chunk(filepath):
    return filepath.read(CHUNK_DATA_SIZE)

def send_single_chunk(socket, filepath):
    socket.send(read_single_chunk(filepath))

def receive_msg(socket) -> str:
    return socket.recv(CHUNK_SIZE)


file_path = sys.argv[1]
address = sys.argv[2]
port = int(sys.argv[3])

socket = socket(AF_INET, SOCK_STREAM)
socket.connect((address, port))

file_name = get_file_name(file_path)
file_size = os.path.getsize(file_path)

chunks_number = ceil(file_size / CHUNK_DATA_SIZE)
chunk_size = CHUNK_DATA_SIZE

