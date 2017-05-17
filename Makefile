# Makefile for sniffer daemon and it's controlling cli

CC=gcc

all: packet_snifferd sniffer

sniffer: sniffer.c statistic.c statistic.h
	$(CC) sniffer.c statistic.c -o sniffer -lsqlite3 -ldl

packet_snifferd: packet_snifferd.c config.c config.h statistic.c statistic.h
	$(CC) packet_snifferd.c config.c statistic.c -o packet_snifferd -lpthread -lsqlite3 -ldl


    
