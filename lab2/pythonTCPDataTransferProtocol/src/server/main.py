import os
import sys
from socket import *

from utils.receiving import *
ADDR = sys.argv[1]
PORT = int(sys.argv[2])

s = socket(AF_INET, SOCK_STREAM)
s.bind((ADDR, PORT))
s.listen(5)

#TODO now it works on single thread with single connection, must work with few threads and few connections later

print("SERVER ACCEPTED CLIENT")
conn, address = s.accept()

f = None
f_size = 0
f_name = ""

print("SERVER ENTERS THE LOOP")
while True:
    msg_json = conn.recv(CHUNK_SIZE)
    print(msg_json.decode("utf-8"))
    msg_obj = Message.from_json(msg_json.decode('utf-8'))
    t = msg_obj.get_message_type()
    if t == "METADATA":
        print("SERVER RECEIVED METADATA FROM CLIENT")
        if not os.path.exists('uploads'):
            os.makedirs('uploads')
        m = Metadata.from_json(msg_json.decode('utf-8'))
        f = open(f"uploads/{m.get_filename()}", 'wb+')
        f_size = m.get_file_size()
        f_name = m.get_filename()
    elif t == "CHUNK":
        print("SERVER RECEIVED CHUNK FROM CLIENT")
        chunk = Chunk.from_json(msg_json.decode('utf-8'))
        if f is not None: #TODO is it good practise write like that???
            dts = handle_chunk(f, chunk).to_json().encode('utf-8')
            print(dts)
            conn.send(dts)
        else:
            raise None
    elif t == "COMPLETE":
        f.close()
        print("SERVER RECEIVED COMPLETE FROM CLIENT")
        print("SERVER CLOSED THE FILE")
        if os.path.getsize(f"uploads/{f_name}") == f_size:
            csuccess = Complete("SUCCESS").to_json().encode('utf-8')
            print(csuccess)
            conn.send(csuccess)
        else:
            cfail = Complete("FAIL").to_json().encode('utf-8')
            print(cfail)
            print(f"uploaded size: {os.path.getsize(f'uploads/{f_name}')}\nf_size: {f_size}")
            conn.send(cfail)
        break

print("SERVER END THE LOOP")