
def handle_chunk(file_object, msg_obj):
    file_object.seek(msg_obj.get('chunk_index'))
    bytes_n = file_object.write(msg_obj.get('data'))
    if bytes_n != len(msg_obj.get("chunk_data_size")):
        return Acknowledge(msg_obj.get('chunk_index'), 'ERROR')
    return Acknowledge(msg_obj.get('chunk_index'), 'OK')


def handle_msg(msg_obj, file_object):
    t = msg_obj.get('message_type')
    if t == 'METADATA':
        file_object = open(f"uploads/{msg_obj.get('file_name')}", "+wb")
        return None
    elif t == 'CHUNK':
        return handle_chunk()
    elif t == 'COMPLETE':
        if msg_obj.get('status') == 'SUCCESS':
            return None
        pass