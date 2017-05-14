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
low-level AF_PACKET socket, reads all the packets in loop
and extract source and destination ip addresses. 

