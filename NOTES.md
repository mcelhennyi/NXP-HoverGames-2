# Notes relating to this project


## Setting up mavlink router
Connect and set `MAV_2_CONFIG` to `TELEM 2` to enable that port.
Then reboot and telem 2 should be active.
This will allow you to use `mavlink-routerd /dev/ttymxc2:921600`
