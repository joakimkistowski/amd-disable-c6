# amd-disable-c6
This is a systemd service to automatically disable the C6 power saving state on AMD Zen (Ryzen / Epyc) processors. The C6 state is known to occasionally freeze Linux distributions running Zen-based processors in deep idle. This systemd service disables the C6 state on boot as a workaround for this bug. Of course, this is only a stop-gap solution until AMD releases a final fix to the underlying issue. 

## Installation

RPM and DEB repositories for the service are [available to be used for apt/dnf/yum/zypper](https://software.opensuse.org//download.html?project=home%3Ajkist&package=amd-disable-c6).  
*Note:* On RPM-based distros you will have to enable and start the service after installation using:
```bash
$ sudo systemctl enable amd_disable_c6.service
$ sudo systemctl start amd_disable_c6.service
```

Supported Distributions at the moment:
* RPM: CentOS 7, Fedora 28, 29, and Rawhide, Mageia Cauldron, openSUSE Tumbleweed, openSUSE Leap 15 and 42.3, RHEL 7, SLE 15
* DEB: Debian 9, Ubuntu 16.04, 17.10, 18.04, 18.10
