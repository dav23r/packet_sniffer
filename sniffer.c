#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <assert.h>

#include <dbus/dbus.h>

#include "packet_snifferd.h"
#include "statistic.h"

static void show_usage();
static void start_daemon();
static void stop_daemon();
static void show_ip_count(char *);
static void select_interface(char *);
static void print_statistics(char *);
static void reset_statistics();
static bool init_dbus_connection(DBusError *, DBusConnection **, 
                                 dbus_uint32_t *, bool check_bus, char *);


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
        select_interface(argv[3]);
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
        return -1;
    }

    return 0;
}

/* Echoes usage information */
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


/* Starts sniffer daemon */
static void start_daemon(){
    printf ("Starting daemon\n");
    // Create dbus structs
    dbus_uint32_t id = 0;
    DBusError error;
    DBusConnection *connection = NULL;
    
    // initialize dbus constructs
    if (!init_dbus_connection(&error, &connection, &id, false, NULL))
        return;
    // Point to the trigger 
    dbus_bus_request_name(connection, DBUS_ACTIVATOR, 0, &error);
    // Create dummy message
    DBusMessage *message;
    message = dbus_message_new_method_call(DBUS_ACTIVATOR, OBJECT_NAME,
                                           INTERFACE, METHOD);
    // 'knock on the door' effectively starting daemon process
    dbus_connection_send (connection, message, &id);
}

/* Stops sniffer daemon */
static void stop_daemon(){
    printf ("Stopping daemon\n");
}

/* Prints num of hits for given ip address */
static void show_ip_count(char *ip){
    connect_to_db();
    printf ("Showing count for %s\n", ip);
    print_ip_count(ip);    
}

static bool init_dbus_connection(DBusError *error, DBusConnection **connection, 
                                 dbus_uint32_t *id, bool check_bus, 
                                 char *well_known_name){
    dbus_error_init (error);
    // connect to the bus
    *connection = dbus_bus_get (DBUS_BUS_SYSTEM, error);
    if (dbus_error_is_set (error)){
        fprintf (stderr, "fatal: Error acquiring system bus %s\n", error->message);
        dbus_error_free (error);
        return false;
    }
    // see if daemon is running
    if (check_bus && !dbus_bus_name_has_owner(*connection, well_known_name, error)){
        fprintf (stderr, "fatal: Daemon not running!! %s\n", error->message);
        dbus_error_free (error);
        return false;
    }

    return true;
}

/* Directs daemon to sniff on given interface */
static void select_interface(char *iface){
    printf ("Making daemon listen to %s\n", iface);
    // Create dbus structs
    dbus_uint32_t id = 0;
    DBusError error;
    DBusConnection *connection = NULL;
    
    // initialize dbus constructs
    if (!init_dbus_connection(&error, &connection, &id, true, DESTINATION))
        return;

    // create message
    DBusMessage *message;
    message = dbus_message_new_method_call(DESTINATION, OBJECT_NAME,
                                           INTERFACE, METHOD);
    if (message == NULL){
        fprintf (stderr, "fatal: Can't initiate method call\n");
    	dbus_error_free (&error);
        return;
    }
    // Add new iface name as argument to message
    dbus_message_append_args (message, DBUS_TYPE_STRING, &iface,
                              DBUS_TYPE_INVALID);
    // Send message
    if (!dbus_connection_send (connection, message, &id)){
        fprintf (stderr, "fatal: Memory overflow");
        dbus_error_free (&error);
        return;
    }

    dbus_message_unref (message);
    dbus_error_free (&error);
}

static void print_statistics(char *iface){
    connect_to_db();
    if (iface == NULL)
        printf ("Showing statistics for all interfaces\n");
    else 
        printf ("Showing statistics for iface %s\n", iface);
    print_all_statistics(iface);
}

static void reset_statistics(){
    connect_to_db();
    reset_db();
}
