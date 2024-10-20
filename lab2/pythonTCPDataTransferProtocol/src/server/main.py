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
        print("SERVER RECEIVED COMPLETE FROM CLIENT")
        break

print("SERVER END THE LOOP")

f.close()

print("SERVER CLOSED FILE")