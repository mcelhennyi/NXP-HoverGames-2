from src.system.controller.python.controller_interface import ControllerInterface, Agent

from threading import Thread, Lock
import time


class ManualController(ControllerInterface):
    def __init__(self):
        ControllerInterface.__init__(self)
        self._running = True

        self._send_position_running = True
        self._position_thread = Thread(target=self._send_position)
        self._agent_location_command_mutex = Lock()
        self._agent_location_command = Agent(0, 0, (0, 0, 0), (0, 0, 0))

    def listen_for_input(self):
        while self._running:
            result = input("Enter a command: ")
            self._process_input(result)

        self._send_position_running = False
        if self._position_thread.isAlive():
            self._position_thread.join()

    def _process_input(self, input_string):

        if input_string == "help":
            print(
                "Manual Controller Commands: \n\n"
                "Position Commands: \n"
                "\tpos start -- Starts commanding positions to the drone.\n"
                "\tpos <agent_id> <x> <y> <z> --  Used to update the position to command the drone to. Note NED: z is negative. "
                "The default is (0,0,-1).\n"
                "\tpos end -- Ends the sending of position commands to the drone.\n"
                "\n"
                "Data Inspection Commands: \n"
                "\tlist agents -- used to list the agents currently available.\n"
                "\tlist subjects -- used to list the subject currently in the scene.\n"
                "\n"
                "Connection to Base Commands: \n"
                "\tconnect - used to connect to the base station.\n"
                # "\tdisconnect - used to shutdown and disconnect from the base station.\n"
                "\n"
                "Shutdown CLI and System: \n"
                "\tshutdown -- used to shutdown this CLI."
            )

        elif input_string.startswith("pos"):
            if input_string == "pos start":
                print("Starting position commanding.")
                self._send_position_running = True
                if not self._position_thread.isAlive():
                    self._position_thread.start()
            elif input_string == "pos end":
                print("Ending position commanding.")
                self._send_position_running = False
            else:
                try:
                    split_string = input_string.split(" ")
                    agent_id = int(split_string[1])
                    x = float(split_string[2])
                    y = float(split_string[3])
                    z = float(split_string[4])

                    # Update the struct
                    self._agent_location_command.set_id(agent_id)
                    self._agent_location_command.update(
                        0,  # Ignored by sender code - ID setting not required
                        (0, 0, 0),  # not used (current position)
                        (x, y, z)   # Target location
                    )

                    print("Updating agent " + str(agent_id) + " location command to: X-" + str(x) + ", Y-" + str(y) +
                          ", Z-" + str(z) + ".")
                except Exception as e:
                    print("Unknown Command '" + input_string + "', try 'help'.")
                    print("Error: " + str(e))

        elif input_string.startswith("list"):
            if input_string == "list agents":
                print("Listing agents: ")
                for agent in self.get_agents():
                    print(agent)
            elif input_string == "list subjects":
                print("Listing subjects: ")
                for subject in self.get_subjects():
                    print(subject)
            else:
                print("Unknown Command '" + input_string + "', try 'help'.")

        elif input_string == "connect":
            print("Connecting to base station...")
            self.start()

        # elif input_string == "disconnect":
        #     print("Disconnecting from base station...")
        #     # TODO: This isnt recoverable
        #     self.shutdown()

        elif input_string == "shutdown":
            print("Shutting down...")
            self.shutdown()
            self._running = False

        elif input_string == "":
            print("\n")

        else:
            print("Unknown command: " + input_string)

    def _send_position(self):
        print("Starting to send position command...")
        while self._send_position_running:
            self._agent_location_command_mutex.acquire()
            self.command_agent_location(self._agent_location_command)
            self._agent_location_command_mutex.release()
            time.sleep(0.1)  # 10 Hz
        print("...ending position command.")


if __name__ == '__main__':
    mc = ManualController()
    mc.listen_for_input()
    print("CLI done.")
