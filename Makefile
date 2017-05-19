# Makefile for sniffer daemon and it's controlling cli

CC=gcc
EXEC_PATH=/usr/bin
DB_PATH=/var/lib
SYSTEMD_UNIT_PATH=/etc/systemd/

all: packet_snifferd sniffer

sniffer: sniffer.c statistic.c statistic.h
	$(CC) sniffer.c statistic.c -o sniffer -lsqlite3 -ldl \
          `pkg-config --libs --cflags dbus-1` \
          -D DB_NAME='"$(DB_PATH)/ips_database"'


packet_snifferd: packet_snifferd.c packet_snifferd.h config.c config.h statistic.c statistic.h 
	$(CC) packet_snifferd.c config.c statistic.c -o packet_snifferd -lpthread -lsqlite3 -ldl \
          `pkg-config --libs --cflags dbus-glib-1` \
          `pkg-config --libs --cflags dbus-1` \
          `pkg-config --libs --cflags glib-2.0` \
          -D DB_NAME='"$(DB_PATH)/ips_database"'

install: sniffer packet_snifferd
	cp packet_snifferd $(EXEC_PATH)
	cp sniffer $(EXEC_PATH) 
	if [ -e ips_database ]; then rm ips_database; fi
	sqlite3 < init.sql
	cp ips_database $(DB_PATH)
	cp packet_snifferd.service $(SYSTEMD_UNIT_PATH)




    
