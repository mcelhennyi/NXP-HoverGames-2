import struct

from src.system.controller.python.messaging.messages import Message, agent_location, header, NODE_TYPE_CONTROLLER, MESSAGE_AGENT_LOCATION, ip_address


class AgentLocation(Message):
    def __init__(self, raw_bytes=None):
        Message.__init__(self, raw_bytes)

        # Parse this message
        self._agent_location_tuple = self.unpack(agent_location)

    def get_agent_id(self):
        return self._agent_location_tuple[0]

    def get_owner_id(self):
        return self._agent_location_tuple[1]

    def get_current_location(self):
        return self._agent_location_tuple[2], self._agent_location_tuple[3], self._agent_location_tuple[4]

    def get_target_location(self):
        return self._agent_location_tuple[5], self._agent_location_tuple[6], self._agent_location_tuple[7]
