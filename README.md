# amd-disable-c6
This is a systemd service to automatically disable the C6 power saving state on AMD Zen (Ryzen / Epyc) processors. The C6 state is known to occasionally freeze Linux distributions running Zen-based processors in deep idle. This systemd service disables the C6 state on boot as a workaround for this bug. Of course, this is only a stop-gap solution until AMD releases a final fix to the underlying issue. 

## Installation

RPM and DEB repositories for the service are [available to be used for apt/dnf/yum/zypper](https://software.opensuse.org//download.html?project=home%3Ajkist&package=amd-disable-c6).  
*Note:* On RPM-based distros you will have to enable and start the service after installation using:
```bash
$ sudo systemctl enable amd-disable-c6.service
$ sudo systemctl start amd-disable-c6.service
```

Supported Distributions at the moment:
* RPM: CentOS 7 and 8, Fedora 28, 29, 30 and Rawhide, Mageia 7 and Cauldron, openSUSE Tumbleweed, openSUSE Leap 15 and 15.1, RHEL 7 and 8, SLE 15 and 15.1
* DEB: Debian 9 and 10, Ubuntu 16.04, 18.04, 18.10 and 19.04

**How to build and install manually:**
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
