# Makefile for sniffer daemon and it's controlling cli

CC=gcc

all: packet_snifferd sniffer

sniffer: sniffer.c
	gcc sniffer.c -o sniffer

packet_snifferd: packet_snifferd.c config.c config.h
	gcc -pthread packet_snifferd.c config.c -o packet_snifferd


    
