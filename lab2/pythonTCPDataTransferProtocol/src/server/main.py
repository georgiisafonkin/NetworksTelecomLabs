import sys
from multiprocessing.connection import answer_challenge
from socket import *

from src.client.utils.receiving import convert_json_to_obj
from src.protocol import *
from src.protocol.messages import CHUNK_SIZE
from src.server.utils.receiving import handle_chunk

ADDR = sys.argv[1]
PORT = sys.argv[2]

s = socket(AF_INET, SOCK_STREAM)
s.bind((ADDR, PORT))
s.listen(5)

#TODO now it works on single thread with single connection, must work with few threads and few connections later

conn, address = s.accept()

f = None

while True:
    msg_json = conn.recv(CHUNK_SIZE)
    msg_obj = convert_json_to_obj(msg_json)
    print(msg_obj)
    t = msg_obj.get('message_type')
    if t == "METADATA":
        f = open(f"uploads/{t.get('filename')}", 'wb+')
    elif t == "CHUNK":
        if f is not None: #TODO is it good practise write like that???
            s.send(handle_chunk(f, msg_obj))
        else:
            raise None
    elif t == "COMPLETE":
        break
