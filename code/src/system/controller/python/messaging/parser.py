import struct

from src.system.controller.python.messaging.messages import *
from src.system.controller.python.messaging.messages.hello import HelloMessage


def create_message(raw_bytes) -> Message:
    message_id = struct.unpack("B", raw_bytes)
    message = None

    if message_id == MESSAGE_ACK:
        message = 0
    elif message_id == MESSAGE_HELLO:
        message = HelloMessage
    elif message_id == MESSAGE_WELCOME:
        pass
    elif message_id == MESSAGE_AGENT_MOVE_COMMAND:
        pass
    elif message_id == MESSAGE_SUBJECT_LOCATION:
        pass
    elif message_id == MESSAGE_AGENT_LOCATION:
        pass
    else:
        print("INVALID MESSAGE ENUM in create_message(raw_bytes)")

    return message
