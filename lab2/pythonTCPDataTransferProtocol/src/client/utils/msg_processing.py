"""useful funcs to handle different types of input messages in a right way"""

from protocol.messages import *
from client.utils import *

def handle_any(msg_obj):
    msg_type = msg_obj.get_message_type()
    if msg_type == "METADATA":
        handle_metadata(msg_obj)
    elif msg_type == "CHUNK":
        handle_chunk(msg_obj)
    elif msg_type == "ACK":
        handle_ack(msg_obj)
    elif msg_type == "COMPLETE":
        handle_complete(msg_obj)
    else:
        raise NotImplementedError(msg_type) #TODO maybe should be replaced to some more convenient later...
    pass

def handle_metadata(msg_obj):
    pass

def handle_chunk(msg_obj):
    pass

def handle_ack(msg_obj):
    chunk_index = msg_obj.get_chunk_index()
    recv_status = msg_obj.get_recv_status()
    if (recv_status == "OK"):
        send_single_chunk() #TODO reading and sent the NEXT chunk
    elif (recv_status == "ERROR"):
        send_single_chunk() #TODO try to send the SAME chunk again

def handle_complete():
    pass