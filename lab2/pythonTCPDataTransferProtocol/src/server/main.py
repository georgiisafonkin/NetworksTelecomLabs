import sys
import json
from socket import *
from utils.receiving import *
ADDR = sys.argv[1]
PORT = int(sys.argv[2])

s = socket(AF_INET, SOCK_STREAM)
s.bind((ADDR, PORT))
s.listen(5)

#TODO now it works on single thread with single connection, must work with few threads and few connections later

conn, address = s.accept()

f = None

while True:
    msg_json = conn.recv(CHUNK_SIZE)
    print(msg_json)
    msg_obj = json.loads(msg_json)
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
