#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <errno.h>

#include <pthread.h>

#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <net/if.h>

#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h> 

#include "config.h"

/* MTU is long enough to accomodate packets of 
   major link layer protocols (ethernet, wlan...) */
#define MTU 5000

static bool set_interface_mask(char *);
static void* listen_cli(void *);

static int socket_fd;

/* Packet sniffer daemon. */
int main(void){

    printf ("[DAEMON] Sniffer daemon about to start\n");

    // Open socket for receiving ip packets
    socket_fd = socket(AF_PACKET, SOCK_DGRAM, htons(ETH_P_ALL));    
    if (socket_fd == -1){
        perror ("[DAEMON] fatal: PACKET socket creation");
        fprintf (stderr, "[DAEMON] Ensure daemon is run with superuser privileges\n");
        return -1;
    }

    // Read configuration file to determine interface to sniff on
    struct config conf;
    get_config(&conf);
    if (!set_interface_mask(conf.if_name)){
        fprintf (stderr, "[DAEMON] fatal: can't bind to interace set in config file\n");
        return -1;
    }

    // Spawn thread which will receive signals from controlling cli
    pthread_t signaller;
    pthread_create (&signaller, NULL, listen_cli, NULL);

    char frame_buffer[MTU];
    while (1) {
        int received_bytes = 
            recv(socket_fd, frame_buffer, sizeof(frame_buffer), 0);
        if (received_bytes == -1){
            perror ("Receiving packet from socket");
        }

        struct iphdr *header = (struct iphdr *) frame_buffer;

        struct in_addr source_addr, dest_addr;
        source_addr.s_addr = header->saddr;
        dest_addr.s_addr = header->daddr;

        printf ("Packet source addr: %s\n", inet_ntoa(source_addr));
        printf ("Packet dest addr: %s\n", inet_ntoa(dest_addr));
    }

    return 0;
}

/* Instructs socket to intercept only packets from particular interface */
static bool set_interface_mask(char *if_name){
    
    printf ("[DAEMON] Directing sinffer on %s interface\n", if_name);

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
    
    return true;
}

/* Listen for dbus messages from controlling cli */
static void* listen_cli(void *data){}

