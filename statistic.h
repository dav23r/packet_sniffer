#ifndef STATISTIC_H
#define STATISTIC_H

#include <sqlite3.h>
#include <stddef.h>
#include <stdio.h>

#define DB_NAME "ips_database"

void connect_to_db();
void add_entry(char *ip_address);
void print_ip_count(char *ip_address);
void print_all_statistics();
void disconnect_from_db();
void reset_db();

#endif
