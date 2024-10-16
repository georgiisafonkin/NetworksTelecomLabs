import os
import sys
from socket import *
from src.client.utils.sending import read_data_in_chunks, create_chunk, convert_to_json, send_single_chunk

CHUNK_DATA_SIZE = 1024
CHUNK_SIZE = 1024 + 512
def get_file_name(filepath):
    return filepath.split('/')[-1]

file_path = sys.argv[1]
address = sys.argv[2]
port = int(sys.argv[3])

socket = socket(AF_INET, SOCK_STREAM)
socket.connect((address, port))

file_name = get_file_name(file_path)
file_size = os.path.getsize(file_path)

f = open(f'{file_path}', 'rb')
g = read_data_in_chunks(f, file_size)

i = 0
while True:
    for chunk in g:
        send_single_chunk(socket, create_chunk(i, chunk))
        socket.recv(CHUNK_SIZE)
