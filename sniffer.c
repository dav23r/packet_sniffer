#include <stdio.h>
#include <string.h>

#include "statistic.h"

static void show_usage();
static void start_daemon();
static void stop_daemon();
static void show_ip_count(char *);
static void select_interface(char *);
static void print_statistics(char *);
static void reset_statistics();


/* Cli controlling daemon that sniffs packets */
int main(int argc, char **argv){

    if (argc < 2 || strcmp(argv[1], "--help") == 0){
        show_usage();
        return -1;
    }

    if (strcmp(argv[1], "start") == 0){
        if (argc != 2){
            show_usage();
            return -1;
        }
        start_daemon();
    } else if (strcmp(argv[1], "stop") == 0){
        if (argc != 2){
            show_usage();
            return -1;
        }
        stop_daemon();
    } else if (strcmp(argv[1], "show") == 0){
        if (argc != 4 || strcmp(argv[3], "count") != 0){
            show_usage();
            return -1;
        }
        show_ip_count(argv[2]);
    } else if (strcmp(argv[1], "select") == 0){
        if (argc != 4 || strcmp(argv[2], "iface") != 0){
            show_usage();
            return -1;
        }
        select_interface(argv[2]);
    } else if (strcmp(argv[1], "stat") == 0){
        char *if_name = NULL;
        if (argc > 3){
            show_usage();
            return -1;
        }
        if (argc == 3)
            if_name = argv[2];
        print_statistics(if_name);
    } else if (strcmp(argv[1], "reset") == 0){
        if (argc != 2){
            show_usage();
            return -1;
        }
        reset_statistics();
    } else {
        show_usage();
    }

    return 0;
}

static void show_usage(){
    printf ("Packet sniffer cli\n");
    printf ("Usage:\n");
    printf ("\t> sniffer COMMAND [IFACE]...\n");
    printf ("\tstart - starts sniffing daemon\n");
    printf ("\tstop - stops sniffing daemon\n");
    printf ("\tshow [ip] count - prints number of packets coming from ip\n");
    printf ("\tselect iface [iface] - directs daemon so sniff packets from interface\n");
    printf ("\treset - deletes all statistics\n");
    printf ("\tstat [iface] - show statistics of inerface, of all if omitted\n");
    printf ("\t--help - output this description message\n");
}


static void start_daemon(){
    printf ("Starting daemon\n");
}

static void stop_daemon(){
    printf ("Stopping daemon\n");
}

static void show_ip_count(char *ip){
    connect_to_db();
    printf ("Showing count for %s\n", ip);
    print_ip_count(ip);    
}

static void select_interface(char *iface){
    printf ("Making daemon listen to %s\n", iface);
}

static void print_statistics(char *iface){
    connect_to_db();
    if (iface == NULL){
        printf ("Showing statistics for all interfaces\n");
        print_all_statistics();
    }
    else {
        printf ("Showing statistics for iface %s\n", iface);
    }
}

static void reset_statistics(){
    connect_to_db();
    reset_db();
}
