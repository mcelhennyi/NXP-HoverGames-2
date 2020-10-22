import socket
import threading

from src.system.controller.python.messaging.parser import create_message


class ControllerInterface:

    def __init__(self):
        # Setup all data this interface needs to own
        self.agents = list()
        self.subjects = list()
        self.my_agents = list()

        # Setup network comms
        self._sock = socket.socket(socket.AF_INET,
                                   socket.SOCK_DGRAM)  # UDP
        self._port = 12345
        self._sock.bind(('', self._port))
        self._running = True
        self._listen_thread = threading.Thread(target=self._listener)

    def start(self):
        """
        Used to start the comms interface

        :return:
        """
        print("Starting up ControllerInterface.")
        self._running = True
        self._listen_thread.start()

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
        # TODO
        pass

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
        # TODO
        pass

    def _listener(self):
        print("Listening on port " + str(self._port) + " for messages.")
        while self._running:
            data, address = self._sock.recvfrom(1024)

            # Parse for a Message
            message = self._on_new_message(data)

            # Now handle that Message
            self._handle_message(message)

    def _on_new_message(self, msg_bytes):
        return create_message(msg_bytes)

    def _handle_message(self, message):
        pass


if __name__ == '__main__':
    ci = ControllerInterface()
    ci.start()

    import time
    while True:
        time.sleep(1)


