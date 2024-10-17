import os
import sys
from socket import *

from src.client.utils.receiving import convert_json_to_obj
from src.client.utils.sending import read_data_in_chunks, create_chunk, convert_to_json, send_single_chunk
from src.protocol.messages import Complete

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

for chunk in g:
    cur_chunk = create_chunk(i, chunk)
    send_single_chunk(socket, cur_chunk)
    msg_obj = convert_json_to_obj(socket.recv(CHUNK_SIZE))
    while msg_obj.get('status') != 'OK':
        send_single_chunk(socket, cur_chunk)
        msg_obj = convert_json_to_obj(socket.recv(CHUNK_SIZE))
    ++i


send_single_chunk(socket, Complete("SUCCESS"))
socket.recv(CHUNK_SIZE)

f.close()
socket.close()
