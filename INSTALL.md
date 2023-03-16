
## Linux Install

Notes about building and installing software in Linux.


```sh
sudo apt install meson
mkdir build
cd build
meson setup
meson compile
meson install
# install will prompt for sudo to install under /usr/local/
```

## configure udev

Find the USB ID.

 - Use `lsusb` to view currently connected USB devices.
 - Plug in Garmin watch.
 - Use `lsusb` to find the newly added USB Garmin device.
 - Look for the ID of the Garmin device

```
lsusb
...
Bus 001 Device 026: ID 091e:2b4b Garmin International
...
```

## Update udev

Update the udev rules file, and install to recognize the Garmin device.

 - Get the VID:PID tuple (like 091e:2b4b) above.
 - Edit the udev rules file and add the new device: extras/10-garmin.rules
 - Install the new udev rule and reload udev

```
cd extras
vi 10-garmin.rules
# add the VID:PID tuple, if not already listed
sudo cp 10-garmin.rules /etc/udev/rules.d/
sudo udevadm control --reload
```

## Read device

Finally, refresh the device connection.

 - Unplug the watch, wait a few seconds, then plug in again.
 - The watch should register as USB flash drive with directories and files.


## NOTES

 - For me, `garmintools` program gives error when trying to open my device.
 - However it is mounted as flash drive with directories and files.


These are just my notes to build/install. Hopefully more will follow.

-Scott 
