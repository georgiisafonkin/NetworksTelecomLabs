"""here some funcs related to recieving data
    parsing json to objects etc."""

import json
from protocol.messages import DATA_SIZE_IN_CHUNK

def receive_msg(socket) -> str:
    return socket.recv(DATA_SIZE_IN_CHUNK)

def convert_json_to_obj(json_str):
    return json.loads(json_str)