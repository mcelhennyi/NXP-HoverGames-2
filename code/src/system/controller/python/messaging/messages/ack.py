import struct

from src.system.controller.python.messaging.messages import Message, ack


class AckMessage(Message):
    def __init__(self, raw_bytes):
        Message.__init__(self, raw_bytes)

        # Parse this message
        self._ack_tuple = struct.unpack(ack, raw_bytes)

    def get_acked_message_timestamp(self):
        return self._hello_tuple[0]
