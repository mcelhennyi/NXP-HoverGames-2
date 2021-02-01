from controller_interface import ControllerInterface

import threading
import time


class ManualController(ControllerInterface):
    def __init__(self):
        # ControllerInterface.__init__(self)
        self._running = True

        self._send_position_running = True
        self._position_thread = threading.Thread(target=self._send_position)
        self._agent_location_command_id = 0
        self._agent_location_command_location = AgentLocation()

    def listen_for_input(self):
        while self._running:
            result = input("Enter a command: ")
            self._process_input(result)

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
                "\tdisconnect - used to shutdown and disconnect from the base station.\n"

            )

        elif input_string.startswith("pos"):
            if input_string == "pos start":
                self._send_position_running = True
                self._position_thread.start()
            elif input_string == "pos end":
                self._send_position_running = False
                self._position_thread.join()  # Wait for it to stop
            else:
                split_string = input_string.split(" ")
                agent_id = int(split_string[1])
                x = float(split_string[2])
                y = float(split_string[3])
                z = float(split_string[4])
                agent_location = AgentLocation()
                print("Updating agent " + str(agent_id) + " location command to :" + str(x) + ", " + str(y) + ", " + str(z) + ".")
        else:
            print("Unknown command: " + input_string)

    def _send_position(self):
        while self._send_position_running:
            self.command_agent_location(self._agent_location_command_id, self._agent_location_command_location)
            time.sleep(0.1)  # 10 Hz


if __name__ == '__main__':
    mc = ManualController()
    mc.listen_for_input()