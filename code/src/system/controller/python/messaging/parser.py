import struct

from src.system.controller.python.messaging.messages import *
from src.system.controller.python.messaging.messages.hello import HelloMessage
from src.system.controller.python.messaging.messages.ack import AckMessage
from src.system.controller.python.messaging.messages.welcome import WelcomeMessage
from src.system.controller.python.messaging.messages.agent_location import AgentLocation


def decode_message(raw_bytes) -> Message:
    message_id = struct.unpack("B", bytearray(raw_bytes[:1]))[0]
    message = None

    if message_id == MESSAGE_WELCOME:
        message = WelcomeMessage(raw_bytes)
    elif message_id == MESSAGE_ACK:
        message = AckMessage(raw_bytes)
    elif message_id == MESSAGE_AGENT_LOCATION:
        message = AgentLocation(raw_bytes)
    # elif message_id == MESSAGE_HELLO:
    #     message = HelloMessage
    # elif message_id == MESSAGE_AGENT_MOVE_COMMAND:
    #     pass
    # elif message_id == MESSAGE_SUBJECT_LOCATION:
    #     pass
    else:
        print("INVALID MESSAGE ENUM, " + str(convert_enum_to_str(message_id)) + ", in create_message(raw_bytes)")

    return message
