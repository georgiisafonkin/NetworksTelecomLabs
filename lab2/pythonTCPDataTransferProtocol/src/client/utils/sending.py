"""here some funcs related to preparation data
    for sending it trough TCP,
    for example, parse objects to json"""

import json

from protocol.messages import *

def read_data_in_chunks(file_object, chunk_size=CHUNK_SIZE):
    while True:
        data = file_object.read(chunk_size)
        if not data:
            break
        yield data

def read_single_chunk(filepath):
    return filepath.read(CHUNK_SIZE)

def send_single_chunk(socket, chunk):
    print(chunk)
    print(chunk.to_json())
    socket.send(chunk.to_json().encode('utf-8'))

def create_chunk(index, data):
    return Chunk(index, data)

def convert_to_json(message):
    return json.dumps(message)