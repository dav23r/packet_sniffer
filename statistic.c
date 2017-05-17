#include "statistic.h"
#include <stdbool.h>

#define MAX_QUERY_SIZE 1000
#define UPDATE_QUERY "update ips set count = count + 1 where ip = '%s'; select * from ips where ip = '%s'"
#define IP_QUERY "select * from ips where ip = '%s'"
#define ADD_QUERY "insert into ips (ip, count) values ('%s', 1)"
#define RESET_QUERY "delete from ips"
#define ALL_QUERY "select * from ips"

static int update_callback(void *data, int num_columns, 
                           char **column_values, char **column_names);

static int show_all_callback(void *data, int num_columns,
                             char **column_values, char **column_names);

static int get_ip_callback(void *data, int num_columns, 
                           char **column_values, char **column_names);

static sqlite3 *db;

void connect_to_db(){
    if (db != NULL)
        fprintf (stderr, "[DATABASE] Reopening existing connection?");

    if (sqlite3_open (DB_NAME, &db) != 0)
        fprintf (stderr, "[DATABASE] Unable to connect");
}

void disconnect_from_db(){
    sqlite3_close(db);
}

void add_entry(char *ip_address){
    char query[MAX_QUERY_SIZE];
    char *error;
    sprintf(query, UPDATE_QUERY, ip_address, ip_address);

    bool updated = false;
    if (sqlite3_exec (db, query, update_callback, (void *) &updated, &error) != 0){
        fprintf (stderr, "[DATABASE] %s at 'add_entry:update'\n", error);
        sqlite3_free(error);
    }
    
    if (!updated) {
        sprintf(query, ADD_QUERY, ip_address);
        if (sqlite3_exec (db, query, NULL, NULL, &error) != 0){
            fprintf (stderr, "[DATABASE] %s at 'add_entry:insert'\n", error);
            sqlite3_free(error);
        }
    }
}

void print_ip_count(char *ip_address){
    char query[MAX_QUERY_SIZE];
    char *error;
    sprintf(query, IP_QUERY, ip_address);
    
    if (sqlite3_exec (db, query, get_ip_callback, NULL, &error) != 0){
        fprintf (stderr, "[DATABASE] %s at 'get_ip_count'\n", error);
        sqlite3_free(error);
    }
}

void print_all_statistics(){
    char query[MAX_QUERY_SIZE];
    char *error;
    sprintf(query, ALL_QUERY);

    if (sqlite3_exec (db, query, show_all_callback, NULL, &error) != 0){
        fprintf (stderr, "[DATABASE] %s at 'show_all_stats'\n", error);
        sqlite3_free(error);
    }
}

void reset_db(){
    char query[MAX_QUERY_SIZE];
    char *error;
    sprintf(query, RESET_QUERY);

    if (sqlite3_exec (db, query, NULL, NULL, &error) != 0){
        fprintf (stderr, "[DATABASE] %s at 'reset'\n", error);
        sqlite3_free(error);
    }

}

static int show_all_callback(void *data, int num_columns,
                             char **column_values, char **column_names){
    char *ip_addr = *column_values;
    char *count_str = *(column_values + 1);
    printf ("%s	%s\n", ip_addr, count_str);
    return 0;
}

static int update_callback(void *data, int num_columns, 
                           char **column_values, char **column_names){
    // Set flag that row with given ip was found
    *(bool *)data = true;
    return 0;
}

static int get_ip_callback(void *data, int num_columns, 
                           char **column_values, char **column_names){
    char *ip_addr = *column_values;
    char *count_str = *(column_values + 1);
    printf ("%s	%s\n", ip_addr, count_str);
    return 0;
}
