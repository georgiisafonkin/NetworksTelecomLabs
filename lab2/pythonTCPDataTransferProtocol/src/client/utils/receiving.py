"""here some funcs related to recieving data
    parsing json to objects etc."""

import json
from src.protocol.messages import *
from src.client.main import CHUNK_SIZE

def receive_msg(socket) -> str:
    return socket.recv(CHUNK_SIZE)

def convert_json_to_obj(json_str):
    return json.loads(json_str)