class Message():
    def __init__(self, __message_type):
        self.__message_type = __message_type

    def get_message_type(self): return self.__message_type

    def set_message_type(self, __message_type): self.__message_type = __message_type

class Metadata(Message):
    def __init__(self, __filename, __file_size, __chunk_size):
        super().__init__("METADATA")
        self.__filename = __filename
        self.__file_size = __file_size
        self.__chunk_size = __chunk_size

    def get_filename(self): return self.__filename

    def get_file_size(self): return self.__file_size

    def get_chunk_size(self): return self.__chunk_size

    def set_filename(self, filename): self.__filename = filename

    def set_file_size(self, file_size): self.__file_size = file_size

    def set_chunk_size(self, chunk_size): self.__chunk_size = chunk_size

class Chunk(Message):
    def __init__(self, __chunk_index, __data):
        super().__init__("CHUNK")
        self.__chunk_index = __chunk_index
        self.__data = __data

    def get_chunk_index(self): return self.__chunk_index

    def get_data(self): return self.__data

    def set_chunk_index(self, __chunk_index): self.__chunk_index = __chunk_index

    def set_data(self, __data): self.__data = __data

class Acknowledge(Message):
    def __init__(self, __chunk_index, __status):
        super().__init__("ACK")
        self.__chunk_index = __chunk_index
        self.__status = __status

    def get_chunk_index(self): return self.__chunk_index

    def get_status(self): return self.__status

    def set_chunk_index(self, __chunk_index): self.__chunk_index = __chunk_index

    def set_status(self, status): self.__status = status

class Complete(Message):
    def __init__(self, __status):
        super().__init__("COMPLETE")
        self.__status = __status

    def get_status(self): return self.__status

    def set_status(self, status): self.__status = status

if __name__ == "__main__":
    a = Metadata("some_file", 1024 * 1000, 1024)
    print(a.__dict__)