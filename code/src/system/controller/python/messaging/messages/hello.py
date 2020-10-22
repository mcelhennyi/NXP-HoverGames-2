import struct

from src.system.controller.python.messaging.messages import Message, hello, header


class HelloMessage(Message):
    def __init__(self, raw_bytes):
        Message.__init__(self, raw_bytes)

        # Parse this message
        self._hello_tuple = self.unpack(hello)

    def get_node_type(self):
        return self._hello_tuple[0]
