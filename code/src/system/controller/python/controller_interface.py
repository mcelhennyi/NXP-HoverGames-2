import socket
import threading

from src.system.controller.python.messaging.parser import decode_message
from src.system.controller.python.messaging.messages.hello import HelloMessage
from src.system.controller.python.messaging.messages.welcome import WelcomeMessage
from src.system.controller.python.messaging.messages.agent_location import AgentLocation
from src.system.controller.python.messaging.messages import *


class Agent:
    def __init__(self, id_, owner_id, location_tuple, target_tuple):
        self._id = id_
        self._owner_id = owner_id

        self._x_location = 0
        self._y_location = 0
        self._z_location = 0

        self._x_target_location = 0
        self._y_target_location = 0
        self._z_target_location = 0

        self.update(owner_id, location_tuple, target_tuple)

    def update(self, owner_id, location_tuple, target_tuple):
        self._x_location = location_tuple[0]
        self._y_location = location_tuple[1]
        self._z_location = location_tuple[2]

        self._x_target_location = target_tuple[0]
        self._y_target_location = target_tuple[1]
        self._z_target_location = target_tuple[2]

        did_change = owner_id is not self._owner_id
        self._owner_id = owner_id

        return did_change


class ControllerInterface:

    def __init__(self, base_ip_address="127.0.0.1", base_port=12345, my_ip_address="127.0.0.1", listening_port=12346):
        # Setup all data this interface needs to own
        self.agents = dict()  # Of type id:Agent
        self.subjects = dict()
        self.my_agents = list()  # list of IDs that can be used to query into above agents dict

        self._my_id = None
        self._base_station_id = None

        # Setup network comms
        self._sock = socket.socket(socket.AF_INET,
                                   socket.SOCK_DGRAM)  # UDP
        self._port = listening_port
        self._sock.bind(('', self._port))

        # Setup send sock
        self._my_ip_address = my_ip_address
        self._base_address = base_ip_address
        self._base_port = base_port

        self._running = True
        self._listen_thread = threading.Thread(target=self._listener)

        self._new_agent_location_callback = None
        self._ownership_change_callback = None

    def start(self):
        """
        Used to start the comms interface

        :return:
        """
        print("Starting up ControllerInterface.")
        self._running = True
        self._listen_thread.start()
        self._connect_to_base()

    def shutdown(self):
        """
        Used to stop the comms interface

        :return:
        """
        print("Shutting down ControllerInterface.")
        self._running = False
        self._listen_thread.join()

    def register_new_agent_location_callback(self, callback):
        """
        The callback will be called when a new agent location is received.

        :param callback:
        :return:
        """
        self._new_agent_location_callback = callback

    def register_new_subject_location_callback(self, callback):
        """
        This callback will be called when a new subject location update occurs.

        :param callback:
        :return:
        """
        # TODO
        pass

    def register_ownership_change_callback(self, callback):
        """
        This callback will be called when ownership of an agent changes.

        :param callback:
        :return:
        """
        self._ownership_change_callback = callback

    def _connect_to_base(self):
        # Connect to base
        while self._my_id is None:
            print("Attempting to connect to base station...")
            hello_msg = HelloMessage(
                source_id=0,
                target_id=0,
                listening_port=self._port,
                ip_address_str=self._my_ip_address
            )
            self._sock.sendto(hello_msg.get_bytes(), (self._base_address, self._base_port))
            time.sleep(1)

    def _listener(self):
        print("Listening on port " + str(self._port) + " for messages.")
        while self._running:
            data, address = self._sock.recvfrom(1024)

            # Parse for a Message
            message = self._on_new_message(data)

            # Now handle that Message
            if message is not None:
                self._handle_message(message)

    def _on_new_message(self, msg_bytes):
        return decode_message(msg_bytes)

    def _handle_message(self, message):
        if message.get_message_id() == MESSAGE_WELCOME:
            self._handle_welcome(message)
        elif message.get_message_id() == MESSAGE_AGENT_LOCATION:
            self._handle_agent_location(message)
        else:
            print("Ignoring message with ID, " + str(message.get_message_id()) + ", for now.")

    def _handle_welcome(self, message):
        # We mark down our ID and the base station ID
        # assert isinstance(message, WelcomeMessage)
        self._my_id = message.get_node_id()
        self._base_station_id = message.get_source_id()

    def _handle_agent_location(self, message):
        assert isinstance(message, AgentLocation)

        agent = None
        ownership_change = False

        # Mark this agents location
        if message.get_agent_id() in self.agents:
            # Old agent, update it
            ownership_change = self.agents[message.get_agent_id()].update(
                message.get_owner_id(),
                message.get_current_location(),
                message.get_target_location()
            )
            agent = self.agents[message.get_agent_id()]

        else:
            # Create a new entry, notify the callbacks
            agent = Agent(
                message.get_agent_id(),
                message.get_owner_id(),
                message.get_current_location(),
                message.get_target_location()
            )
            self.agents[message.get_agent_id()] = agent
            ownership_change = True  # Always when its new

        # Update the my_agents list
        if message.get_owner_id() == self._my_id:
            if message.get_agent_id() not in self.my_agents:
                # if we do not have the agent in my agents, add it
                self.my_agents.append(message.get_agent_id())
        else:
            # We dont own it, make sure ownership isnt in our list
            if message.get_agent_id() in self.my_agents:
                # Its in here, lets remove it
                self.my_agents.remove(message.get_agent_id())

        # Notify the callback
        if self._new_agent_location_callback is not None:
            self._new_agent_location_callback(agent)

        # Notify of ownership change
        if self._ownership_change_callback is not None and ownership_change:
            self._ownership_change_callback(agent)

    def _handle_ack(self, message):
        # TODO: Skip for now, assume success
        pass

    def _handle_subject_location(self, message):
        # TODO Unneeded for now since we are only going to have one controller
        pass


if __name__ == '__main__':
    ci = ControllerInterface()
    ci.start()

    import time
    running = True
    try:
        while running:
            time.sleep(1)
    except KeyboardInterrupt as ke:
        print("Shutting down system...")
        ci.shutdown()
        exit(0)
