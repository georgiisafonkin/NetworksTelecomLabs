"""here some funcs related to preparation data
    for sending it trough TCP,
    for example, parse objects to json"""

from protocol.messages import *

def read_data_in_chunks(file_object, chunk_size=DATA_SIZE_IN_CHUNK):
    while True:
        data = file_object.read(chunk_size)
        if not data:
            break
        yield data

def send_msg(socket, msg):
    s_msg = msg.to_json().encode('utf-8')
    socket.send(msg.to_json().encode('utf-8'))