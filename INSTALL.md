
# Linux Install

```
sudo apt install meson
mkdir build
cd build
meson setup
meson compile
meson install
# install will prompt for sudo to install under /usr/local/
```

# configure udev

plug in garmin device.  then get specific usb vendor and product id values.

Plug in Garmin watch.
`lsusb`

look for VID:PID tuple

edit the extras udev rule file to add your device

```
cd extras
vi 10-garmin.rules
# add the VID:PID tuple, if not already listed
sudo cp 10-garmin.rules /etc/udev/rules.d/
sudo udevadm control --reload
```

unplug the watch, wait a few seconds, then plug in again.

the watch should register as USB flash drive with directories and files.

NOTE: garmintools not able to access the device.

