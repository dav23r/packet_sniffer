#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <errno.h>

#include <pthread.h>
#include <signal.h>

#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <net/if.h>

#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h> 
#include <unistd.h>

#include <glib.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus.h>

#include "config.h"
#include "statistic.h"
#include "packet_snifferd.h"

/* MTU is long enough to accomodate packets of 
   major link layer protocols (ethernet, wlan...) */
#define MTU 5000

#define MAX_FILTER_SIZE 200

static bool set_interface_mask(char *);
static void* listen_cli(void *);

static int socket_fd;
static struct config *conf;
static DBusConnection *connection;

/* Terminate cleanly */
void sig_term_handler(int sig_num){
    if (socket_fd) close(socket_fd);
    if (conf != NULL) {config_dispose(conf); free(conf);}
    dbus_connection_flush(connection);
    printf ("SIGTERM recieved, stopping daemon gracefully\n");
    exit (0);
}

/* Reload configuration on SIGHUP */
void sig_hup_handler(int sig_num){
    if (conf != NULL) {
        config_dispose(conf); 
        free(conf);
    }
    conf = get_config();
    printf ("Configuration reloaded after recieving SIGHUP");
}


/* Packet sniffer daemon. */
int main(void){

    printf ("[DAEMON] Sniffer daemon about to start\n");
    signal (SIGTERM, sig_term_handler);
    signal (SIGHUP, sig_hup_handler);
    signal (SIGINT, sig_term_handler);

    // Open socket for receiving ip packets
    socket_fd = socket(AF_PACKET, SOCK_DGRAM, htons(ETH_P_ALL));    
    if (socket_fd == -1){
        perror ("[DAEMON] fatal: PACKET socket creation");
        fprintf (stderr, "[DAEMON] Ensure daemon is run with superuser privileges\n");
        return 1;
    }

    // Read configuration file to determine interface to sniff on
    conf = get_config();
    if (!set_interface_mask(conf->if_name)){
        fprintf (stderr, "[DAEMON] fatal: can't bind to interface set in config file\n");
        return 1;
    }

    // Connect to database to be able to store statistics
    connect_to_db();

    // Spawn thread which will receive signals from controlling cli
    pthread_t signaller;
    pthread_create (&signaller, NULL, listen_cli, NULL);

    char frame_buffer[MTU];
    struct sockaddr_ll packet_frame_info;
    int packet_frame_info_len = sizeof packet_frame_info;
    while (1) {
        int received_bytes = recvfrom(socket_fd, frame_buffer, sizeof(frame_buffer), 0, 
                                      (struct sockaddr *) &packet_frame_info, &packet_frame_info_len);
        if (received_bytes == -1){
            perror ("Receiving packet from socket");
        }

        // We are interested in only incoming packets
        if (packet_frame_info.sll_pkttype != PACKET_HOST)
            continue;

        struct iphdr *header = (struct iphdr *) frame_buffer;

        struct in_addr source_addr, dest_addr;
        source_addr.s_addr = header->saddr;
        dest_addr.s_addr = header->daddr;

        add_entry(inet_ntoa(source_addr), conf->if_name);
    }

    return 0;
}

/* Instructs socket to intercept only packets from particular interface */
static bool set_interface_mask(char *if_name){
    
    printf ("[DAEMON] Directing sniffer on '%s' interface\n", if_name);

    int if_index = if_nametoindex(if_name);
    if (if_index == 0){
        fprintf (stderr, "[DAEMON] No such device in the system %s\n", if_name);
        return false;
    }

    struct sockaddr_ll if_mask;
    memset(&if_mask, 0, sizeof if_mask);

    if_mask.sll_protocol = htons(ETH_P_ALL);
    if_mask.sll_ifindex = if_index;
    if_mask.sll_family = AF_PACKET;

    int ret = bind (socket_fd, 
		    (const struct sockaddr *) &if_mask, sizeof if_mask); 
    if (ret == -1){
        perror("[DAEMON] Binding socket to interface");
        return false;
    }

    free(conf->if_name);
    conf->if_name = strdup(if_name);
    
    return true;
}


static DBusHandlerResult message_receiver(DBusConnection *connection, 
                                          DBusMessage *message, void *aux){
    GMainLoop *main_loop = aux;
    DBusError error;

    if (dbus_message_is_method_call(message, INTERFACE, METHOD)){
        dbus_error_init (&error);
        char *iface_name;
        if (!dbus_message_get_args (message, &error, DBUS_TYPE_STRING, 
                                    &iface_name, DBUS_TYPE_INVALID)){
            fprintf (stderr, "[DAEMON] Can't get argument of dbus method call\n");
            dbus_error_free (&error);
        } else {
            if (!set_interface_mask(iface_name)){
                fprintf (stderr, "[DAEMON] No iterface with name %s\n", iface_name);
            }
        }
    }
    return DBUS_HANDLER_RESULT_HANDLED;
}

/* Listen for dbus messages from controlling cli */
static void* listen_cli(void *data){
    GMainLoop *loop;
    DBusError error;

    loop = g_main_loop_new (NULL, FALSE);

    dbus_error_init (&error);
    connection = dbus_bus_get (DBUS_BUS_SESSION, &error);
    
    if (connection == NULL) {
        fprintf (stderr, "[DAEMON] fatal: Unable to acquire session bus\n");
        dbus_error_free (&error);
        exit(1);
    }

    dbus_bus_request_name(connection, DESTINATION, 0, &error);
    if (dbus_error_is_set(&error)){
        fprintf (stderr, "[DAEMON] fatal: Name error %s\n", error.message);
        dbus_error_free (&error);
        exit(1);
    }
    dbus_connection_setup_with_g_main (connection, NULL);

    /* listening to messages from all objects as no path is specified */
    char filter[MAX_FILTER_SIZE];
    sprintf(filter, "path='%s',type='%s',interface='%s'", OBJECT_NAME, "method_call", INTERFACE);
    
    dbus_bus_add_match (connection, filter, &error);
    if (dbus_error_is_set(&error)){
        fprintf (stderr, "Error setting match: %s\n", error.message);
        exit(1);
    }
    dbus_connection_flush(connection);
    dbus_connection_add_filter (connection, message_receiver, loop, NULL);

    g_main_loop_run (loop);
    return NULL;
}

