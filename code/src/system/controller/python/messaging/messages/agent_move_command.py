import struct

from src.system.controller.python.messaging.messages import Message, agent_move_command, header, NODE_TYPE_CONTROLLER, MESSAGE_AGENT_MOVE_COMMAND, ip_address


class AgentMoveCommand(Message):
    def __init__(self,
                 # from  raw bytes
                 raw_bytes=None,

                 # Constructed as command
                 agent_id=None,
                 source_id=None,
                 commanded_location=None
                 ):
        Message.__init__(self, raw_bytes, message_id=MESSAGE_AGENT_MOVE_COMMAND, source_id=source_id, target_id=agent_id)

        if raw_bytes is None:
            # No bytes, we are making a message
            self._agent_move_command_tuple = (agent_id, commanded_location[0], commanded_location[1], commanded_location[2])
        else:
            # Parse this message
            self._agent_move_command_tuple = self.unpack(agent_move_command)

    def get_agent_id(self):
        return self._agent_move_command_tuple[0]

    def set_agent_id(self, agent_id):
        self._agent_move_command_tuple[0] = agent_id

    def get_target_location(self):
        return self._agent_move_command_tuple[1], self._agent_move_command_tuple[2], self._agent_move_command_tuple[3]

    def get_bytes(self):
        return Message.get_bytes(self) + struct.pack(
            agent_move_command,

            # Agent id
            self._agent_move_command_tuple[0],

            # Target location
            self._agent_move_command_tuple[1],
            self._agent_move_command_tuple[2],
            self._agent_move_command_tuple[3],
        )