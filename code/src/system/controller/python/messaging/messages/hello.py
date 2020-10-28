import struct

from src.system.controller.python.messaging.messages import Message, hello, header, NODE_TYPE_CONTROLLER, MESSAGE_HELLO, ip_address


class HelloMessage(Message):
    def __init__(self,
                 raw_bytes=None,
                 # OR
                 source_id=None, target_id=None, listening_port=None, ip_address_str=None):
        Message.__init__(self, raw_bytes, message_id=MESSAGE_HELLO, source_id=source_id, target_id=target_id)

        if raw_bytes is None:
            ip_address_split = ip_address_str.split('.')
            # ip_address_packed = struct.pack(ip_address,
            #                                 int(ip_address_split[0]),
            #                                 int(ip_address_split[1]),
            #                                 int(ip_address_split[2]),
            #                                 int(ip_address_split[3])
            #                                 )
            self._hello_tuple = (NODE_TYPE_CONTROLLER, listening_port, int(ip_address_split[0]), int(ip_address_split[1]), int(ip_address_split[2]), int(ip_address_split[3]))
        else:
            # Parse this message
            self._hello_tuple = self.unpack(hello)

    def get_bytes(self):
        return Message.get_bytes(self) + struct.pack(
            hello,

            # Node type
            self._hello_tuple[0],

            # Listening Port
            self._hello_tuple[1],

            # IP Below
            self._hello_tuple[2],
            self._hello_tuple[3],
            self._hello_tuple[4],
            self._hello_tuple[5]
        )
