import struct

from src.system.controller.python.messaging.messages import Message, agent_location, header, NODE_TYPE_CONTROLLER, MESSAGE_AGENT_LOCATION, ip_address


class AgentLocation(Message):
    def __init__(self,
                 # From raw bytes
                 raw_bytes=None,

                 # Constructed as command
                 agent_id=None,
                 owner_id=None,
                 commanded_location=None
                 ):
        Message.__init__(self, raw_bytes, message_id=MESSAGE_AGENT_LOCATION, source_id=owner_id, target_id=agent_id)

        if raw_bytes is None:
            # No bytes, we are making a message
            self._agent_location_tuple = (agent_id, owner_id,  0, 0, 0, commanded_location[0], commanded_location[1], commanded_location[2])
        else:
            # Parse this message
            self._agent_location_tuple = self.unpack(agent_location)

    def get_agent_id(self):
        return self._agent_location_tuple[0]

    def set_agent_id(self, agent_id):
        self._agent_location_tuple[0] = agent_id

    def get_owner_id(self):
        return self._agent_location_tuple[1]

    def get_current_location(self):
        return self._agent_location_tuple[2], self._agent_location_tuple[3], self._agent_location_tuple[4]

    def get_target_location(self):
        return self._agent_location_tuple[5], self._agent_location_tuple[6], self._agent_location_tuple[7]

    def get_bytes(self):
        return Message.get_bytes(self) + struct.pack(
            agent_location,

            # Agent id
            self._agent_location_tuple[0],

            # Onwer ID
            self._agent_location_tuple[1],

            # Current location
            self._agent_location_tuple[2],
            self._agent_location_tuple[3],
            self._agent_location_tuple[4],

            # Target location
            self._agent_location_tuple[5],
            self._agent_location_tuple[6],
            self._agent_location_tuple[7],
        )