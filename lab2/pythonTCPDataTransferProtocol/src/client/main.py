import os
import sys
from socket import *

from utils.receiving import convert_json_to_obj
from utils.sending import read_data_in_chunks, create_chunk, convert_to_json, send_msg
from protocol.messages import *

def get_file_name(filepath):
    return filepath.split('/')[-1]


file_path = sys.argv[1]
address = sys.argv[2]
port = int(sys.argv[3])

socket = socket(AF_INET, SOCK_STREAM)
socket.connect((address, port))

file_name = get_file_name(file_path)
file_size = os.path.getsize(file_path)

send_msg(socket, Metadata(file_name, file_size, DATA_SIZE_IN_CHUNK))

f = open(f'{file_path}', 'rb+')
g = read_data_in_chunks(f, DATA_SIZE_IN_CHUNK)

i = 0
for chunk in g:
    cur_chunk = create_chunk(i, chunk)
    # print(cur_chunk.to_json())
    send_msg(socket, cur_chunk)
    received_json_str = b''
    while received_json_str == b'':
        received_json_str = socket.recv(CHUNK_SIZE)
    print("received: " + received_json_str.decode("utf-8"))
    ack_obj = Acknowledge.from_json(received_json_str.decode('utf-8'))
    while ack_obj.get('status') != 'OK':
        print(cur_chunk)
        send_msg(socket, cur_chunk)
        ack_obj = convert_json_to_obj(socket.recv(CHUNK_SIZE))
    i += 1


send_msg(socket, Complete("SUCCESS"))
socket.recv(DATA_SIZE_IN_CHUNK)

f.close()
socket.close()
