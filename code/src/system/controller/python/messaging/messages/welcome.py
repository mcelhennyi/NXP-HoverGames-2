import struct

from src.system.controller.python.messaging.messages import Message, welcome, header, NODE_TYPE_CONTROLLER, MESSAGE_WELCOME, ip_address


class WelcomeMessage(Message):
    def __init__(self, raw_bytes=None):
        Message.__init__(self, raw_bytes)

        # Parse this message
        self._welcome_tuple = self.unpack(welcome)

    def get_node_id(self):
        return self._header_tuple[0]
