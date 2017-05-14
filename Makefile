# Makefile for sniffer daemon and it's controlling cli

CC=gcc

packet_snifferd: packet_snifferd.c
	gcc packet_snifferd.c -o packet_snifferd


    
