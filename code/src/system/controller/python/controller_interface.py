import socket
import threading

from typing import Dict, List, Callable

from src.system.controller.python.messaging.parser import decode_message
from src.system.controller.python.messaging.messages.hello import HelloMessage
from src.system.controller.python.messaging.messages.welcome import WelcomeMessage
from src.system.controller.python.messaging.messages.agent_location import AgentLocation
from src.system.controller.python.messaging.messages.agent_move_command import AgentMoveCommand
from src.system.controller.python.messaging.messages import *


class Subject:
    def __init__(self, owner_id, location_tuple, target_tuple):
        self._id = 0
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

    def set_id(self, id_):
        # This is assigned by the interface, not the user!
        self._id = id_

    def get_id(self):
        return self._id

    def get_location(self):
        return self._x_location, self._y_location, self._z_location

    def get_destination(self):
        return self._x_target_location, self._y_target_location, self._z_target_location

    def get_owner(self):
        return self._owner_id


class Agent(Subject):
    def __init__(self, id_, owner_id, location_tuple, target_tuple):
        Subject.__init__(self, owner_id, location_tuple, target_tuple)
        self.set_id(id_)


AgentDict = Dict[int, Agent]
AgentList = List[Agent]
SubjectDict = Dict[int, Subject]


class ControllerInterface:

    def __init__(self, base_ip_address="127.0.0.1", base_port=12345, my_ip_address="127.0.0.1", listening_port=12346):
        # Setup all data this interface needs to own
        self._agents = dict()  # Of type id:Agent
        self._subjects = dict()
        self._my_agents = list()  # list of IDs that can be used to query into above agents dict

        self._my_id = None
        self._base_station_id = None

        # Setup network comms
        self._sock = socket.socket(socket.AF_INET,
                                   socket.SOCK_DGRAM)  # UDP
        self._port = listening_port
        self._sock.bind(('', self._port))
        self._sock.settimeout(1)

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

    def register_new_agent_location_callback(self, callback: Callable[[Agent], None]):
        """
        The callback will be called when a new agent location is received.

        :param callback:
        :return:
        """
        self._new_agent_location_callback = callback

    # def register_new_subject_location_callback(self, callback: Callable[Subject]):
    #     """
    #     This callback will be called when a new subject location update occurs.
    #
    #     :param callback:
    #     :return:
    #     """
    #     # TODO - removed for now since the user of this class will be the single source of subjects - for now.
    #     pass

    def register_ownership_change_callback(self, callback: Callable[[Agent], None]):
        """
        This callback will be called when ownership of an agent changes.

        :param callback:
        :return:
        """
        self._ownership_change_callback = callback

    def get_id(self):
        if self._my_id is None:
            print("WARNING: ID not set yet, please connect to base station first.")
        return self._my_id

    def get_agents(self) -> AgentDict:
        return self._agents

    def get_my_agents(self) -> AgentList:
        return self._my_agents

    def get_subjects(self) -> SubjectDict:
        return self._subjects

    def add_new_subject(self, subject: Subject):
        # Create a new subject ID
        # TODO: This needs to be either namespaced or more random in the multibase case. Currently with one base we
        #  will just increment.

        # Current ids in a list, so we can make a new id
        current_ids = [old_id for old_id in self._subjects.keys()]

        # iterate over list and find the largest
        current_ids.sort()
        new_subject_id = max(current_ids)

        # Create a new subject
        subject.set_id(new_subject_id)
        self._subjects[new_subject_id] = subject

        return new_subject_id

    def command_agent_location(self, agent: Agent):
        al = AgentMoveCommand(
            agent_id=agent.get_id(), source_id=self._my_id, commanded_location=agent.get_destination()
        )
        self._sock.sendto(al.get_bytes(), (self._base_address, self._base_port))

# -- Privates Below -- #

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
        print("Connected! Given ID: " + str(self._my_id))

    def _listener(self):
        print("Listening on port " + str(self._port) + " for messages.")
        while self._running:
            try:
                data, address = self._sock.recvfrom(1024)

                # Parse for a Message
                if data:
                    message = self._on_new_message(data)

                    # Now handle that Message
                    if message is not None:
                        self._handle_message(message)
            except socket.timeout as to:
                pass

    def _on_new_message(self, msg_bytes):
        return decode_message(msg_bytes)

    def _handle_message(self, message: Message):
        if message.get_message_id() == MESSAGE_WELCOME:
            self._handle_welcome(message)
        elif message.get_message_id() == MESSAGE_AGENT_LOCATION:
            self._handle_agent_location(message)
        else:
            print("Ignoring message with ID, " + str(message.get_message_id()) + ", for now.")

    def _handle_welcome(self, message: WelcomeMessage):
        # We mark down our ID and the base station ID
        # assert isinstance(message, WelcomeMessage)
        self._my_id = message.get_node_id()
        self._base_station_id = message.get_source_id()

    def _handle_agent_location(self, message: Message):
        assert isinstance(message, AgentLocation)

        agent = None
        ownership_change = False

        # Mark this agents location
        if message.get_agent_id() in self._agents:
            # Old agent, update it
            ownership_change = self._agents[message.get_agent_id()].update(
                message.get_owner_id(),
                message.get_current_location(),
                message.get_target_location()
            )
            agent = self._agents[message.get_agent_id()]

        else:
            # Create a new entry, notify the callbacks
            agent = Agent(
                message.get_agent_id(),
                message.get_owner_id(),
                message.get_current_location(),
                message.get_target_location()
            )
            self._agents[message.get_agent_id()] = agent
            ownership_change = True  # Always when its new

        # Update the my_agents list
        if message.get_owner_id() == self._my_id:
            if message.get_agent_id() not in self._my_agents:
                # if we do not have the agent in my agents, add it
                self._my_agents.append(message.get_agent_id())
        else:
            # We dont own it, make sure ownership isnt in our list
            if message.get_agent_id() in self._my_agents:
                # Its in here, lets remove it
                self._my_agents.remove(message.get_agent_id())

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
