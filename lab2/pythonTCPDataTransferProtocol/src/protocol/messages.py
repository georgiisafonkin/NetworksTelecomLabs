import base64
import json

CHUNK_SIZE = 8192
DATA_SIZE_IN_CHUNK = 32 # 1 KiB

class Message():
    def __init__(self, __message_type):
        self.__message_type = __message_type

    def to_json(self):
        return json.dumps({
            'message_type': self.get_message_type(),
        }, sort_keys=True, indent=4)

    @staticmethod
    def from_json(json_str):
        obj = json.loads(json_str)
        message_type = obj['message_type']
        return Message(message_type)

    def get_message_type(self): return self.__message_type

    def set_message_type(self, __message_type): self.__message_type = __message_type

class Metadata(Message):
    def __init__(self, __filename, __file_size, __chunk_size):
        super().__init__("METADATA")
        self.__filename = __filename
        self.__file_size = __file_size
        self.__chunk_size = __chunk_size #TODO remove field, because unused

    def to_json(self):
        return json.dumps({
            'message_type': self.get_message_type(),
            'filename': self.__filename,
            'file_size': self.__file_size,
            'chunk_size': self.__chunk_size,
        }, sort_keys=True, indent=4)


    @staticmethod
    def from_json(json_str):
        obj = json.loads(json_str)
        filename = obj['filename']
        file_size = obj['file_size']
        chunk_size = obj['chunk_size']
        return Metadata(filename, file_size, chunk_size)

    def get_filename(self): return self.__filename

    def get_file_size(self): return self.__file_size

    def get_chunk_size(self): return self.__chunk_size

    def set_filename(self, filename): self.__filename = filename

    def set_file_size(self, file_size): self.__file_size = file_size

    def set_chunk_size(self, chunk_size): self.__chunk_size = chunk_size

class Chunk(Message):
    def __init__(self, __chunk_index, __data, __data_bytes):
        super().__init__("CHUNK")
        self.__chunk_index = __chunk_index
        self.__data = __data
        self.__data_bytes = __data_bytes

    def to_json(self):
        if isinstance(self.__data, bytes):
            encoded_data = base64.b64encode(self.__data).decode('utf-8')
        else:
            encoded_data = self.__data

        return json.dumps({
            'message_type': self.get_message_type(),
            'chunk_index': self.__chunk_index,
            'data': encoded_data,
            'data_bytes': self.__data_bytes
        }, sort_keys=True, indent=4)

    @staticmethod
    def from_json(json_str):
        obj = json.loads(json_str)
        chunk_index = obj['chunk_index']
        data = base64.b64decode(obj['data']) if isinstance(obj['data'], str) else obj['data']
        data_bytes = len(data)
        return Chunk(chunk_index, data, data_bytes)

    def get_chunk_index(self): return self.__chunk_index

    def get_data(self): return self.__data

    def get_data_bytes(self): return self.__data_bytes

    def set_chunk_index(self, __chunk_index): self.__chunk_index = __chunk_index

    def set_data(self, __data): self.__data = __data


class Acknowledge(Message):
    def __init__(self, __chunk_index, __status):
        super().__init__("ACK")
        self.__chunk_index = __chunk_index
        self.__status = __status

    def to_json(self):
        return json.dumps({
            'message_type': self.get_message_type(),
            'chunk_index': self.__chunk_index,
            'status': self.__status
        }, sort_keys=True, indent=4)


    @staticmethod
    def from_json(json_str):
        obj = json.loads(json_str)
        chunk_index = obj['chunk_index']
        status = obj['status']
        return Acknowledge(chunk_index, status)

    def get_chunk_index(self): return self.__chunk_index

    def get_status(self): return self.__status

    def set_chunk_index(self, __chunk_index): self.__chunk_index = __chunk_index

    def set_status(self, status): self.__status = status

class Complete(Message):
    def __init__(self, __status):
        super().__init__("COMPLETE")
        self.__status = __status

    def to_json(self):
        return json.dumps({
            'message_type': self.get_message_type(),
            'status': self.__status
        }, sort_keys=True, indent=4)


    @staticmethod
    def from_json(json_str):
        obj = json.loads(json_str)
        status = obj['status']
        return Complete(status)

    def get_status(self): return self.__status

    def set_status(self, status): self.__status = status

if __name__ == "__main__":
    a = Metadata("some_file", 1024 * 1000, 1024)
    print(a.__dict__)