#ifndef STATISTIC_H
#define STATISTIC_H

#include <sqlite3.h>
#include <stddef.h>
#include <stdio.h>

void connect_to_db();
void add_entry(char *ip_address, char *iface);
void print_ip_count(char *ip_address);
void print_all_statistics(char *iface);
void disconnect_from_db();
void reset_db();

#endif
