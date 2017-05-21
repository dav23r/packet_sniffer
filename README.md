# packet_sniffer

The document describes implementation of Raw ip packet sniffer 
daemon and it's cli controlling suite.

snifferd architecture complies with modern linux daemon
specifications. It integrates with 'systemd' init system.
Comunication between cli and daemon are performed via
DBus interface. 

Daemon flow:
Since sniffer should continuously monitor traffic in the 
background and at the same time respond to signals from cli
and change behaviour accordingly, the two metioned tasks are
broken into different threads. 
Sniffer spawns a 'receiver' thread which listends to DBus
messages (from cli). It then proceeds with creation of 
low-level AF_PACKET socket, reads all incoming packets in loop
and extract source and destination ip addresses. 
Receiver thread declares it's 'well-known-name' on dbus system
bus and waits for messages (in blocking manner). It associates
callback with incoming message. 
There are two types of dbus messages handled:
1) CHANGE_IFACE - signal to direct monitor to different iface
2) STOP - stop daemon


In order to start inactive daemon, signal is sent to ACTIVATION_BUS 
dbus 'well-know-name'. When signal is received, dbus starts 
'packet_snifferd' systemd service (i.e service is dbus activatable).


One can control daemon itself from systemd's systemctl

```bash
systemctl start packet_snifferd.service
systemctl status packet_snifferd.service
systemctl stop packet_snifferd.service
```

Sniffer (cli) can be used to start/stop daemon as well.
```bash
sudo sniffer start
sudo sniffer stop
```

Note, all invocations of 'sniffer' except showing statistics
require superuser privileges since signal should be sent over
dbus system bus.

Data internally is stored in sqlite database and accessed with
sqlite's c api.
In order to install program on your linux machine, cd to source
code directory and issue:
sudo make install

Before first start of daemon, you should create
file in /etc/packet_snifferd/conf
which will contain line
```
iface="your default iface name to monitor"
```
for example:
```
iface=wlan0
```

If something goes wrong, you may need to tweak path variables in 
the beginning of Makefile. With current settings the program is
successfully deployed on Ubuntu 17.04 machine.

EXAMPLE SESSION:
```bash
dav23r@ubuntu-s300ca:~$ sudo sniffer start
Starting daemon
Done
dav23r@ubuntu-s300ca:~$ systemctl status packet_snifferd.service 
● packet_snifferd.service - Ip packet sniffer daemon
   Loaded: loaded (/etc/systemd/system/packet_snifferd.service; static; vendor preset: enabled)
   Active: active (running) since Sun 2017-05-21 21:21:13 +04; 1s ago
 Main PID: 6758 (packet_snifferd)
    Tasks: 2 (limit: 4915)
   Memory: 824.0K
      CPU: 6ms
   CGroup: /system.slice/packet_snifferd.service
           └─6758 /usr/bin/packet_snifferd

May 21 21:21:13 ubuntu-s300ca systemd[1]: Started Ip packet sniffer daemon.
dav23r@ubuntu-s300ca:~$ sniffer stat
Showing statistics for all interfaces
52.35.173.218	1
192.168.0.1	20
162.125.18.133	2
dav23r@ubuntu-s300ca:~$ sudo sniffer select iface lo
Making daemon listen to lo
```

DEPENDENCIES:
```
systemd - init system
dbus - ipc mechanism 
glib - c api for dbus
sqlite3 - serverless database 
```
