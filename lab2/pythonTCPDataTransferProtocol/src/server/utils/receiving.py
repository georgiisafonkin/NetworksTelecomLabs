from protocol.messages import *

def handle_chunk(file_object, chunk_obj):
    file_object.seek(chunk_obj.get_chunk_index() * DATA_SIZE_IN_CHUNK)
    bytes_n = file_object.write(chunk_obj.get_data())
    if bytes_n != chunk_obj.get_data_bytes():
        return Acknowledge(chunk_obj.get_chunk_index(), 'ERROR')
    return Acknowledge(chunk_obj.get_chunk_index(), 'OK')

def handle_metadata(file_object, metadata):
    file_object = open(f"uploads/{metadata.get_filename()}", "+wb")
    return None