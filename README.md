# amd-disable-c6
This is a systemd service to automatically disable the C6 power saving state on AMD Zen (Ryzen / Epyc) processors. The C6 state is known to occasionally freeze Linux distributions running Zen-based processors in deep idle. This systemd service disables the C6 state on boot as a workaround for this bug. Of course, this is only a stop-gap solution until AMD releases a final fix to the underlying issue. 

## How to build and install

Checkout this repository or download the latest release sources.

Then, from inside the source directory run:

```bash
$ sudo make install
$ sudo systemctl enable amd-disable-c6.service
$ sudo systemctl start amd-disable-c6.service
```

## Troubleshooting
The most likely reason for the service to fail is the `msr` module not being present. It should be present on most distros, but some (e.g., Arch, Manjaro) do not have it enabled by default.

To enable the module:
```bash
$ sudo modprobe msr #enable until next reboot
$ sudo sh -c "echo msr > /etc/modules-load.d/msr.conf" #auto-enable at boot time
```
