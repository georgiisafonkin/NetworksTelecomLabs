"""here some funcs related to preparation data
    for sending it trough TCP,
    for example, parse objects to json"""

import json

from protocol.messages import *

def read_data_in_chunks(file_object, chunk_size=DATA_SIZE_IN_CHUNK):
    while True:
        data = file_object.read(chunk_size)
        if not data:
            break
        yield data

def send_msg(socket, msg):
    socket.send(msg.to_json().encode('utf-8'))

def create_chunk(index, data):
    return Chunk(index, data)

def convert_to_json(message):
    return json.dumps(message)