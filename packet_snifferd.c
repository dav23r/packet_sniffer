#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <errno.h>

#include <pthread.h>

#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h> 

/* MTU is long enough to accomodate packets of 
   major link layer protocols (ethernet, wlan...) */
#define MTU 5000

static void set_interface_mask(int);
static void* listen_cli(void *);

static int socket_fd;

/* Packet sniffer daemon. */
int main(void){

    // Open socket for receiving ip packets
    socket_fd = socket(AF_PACKET, SOCK_DGRAM, htons(ETH_P_ALL));    
    if (socket_fd == -1){
        perror ("PACKET socket creation");
        fprintf (stderr, "Ensure daemon is run with superuser privileges\n");
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
static void set_interface_mask(int if_index){
    
    struct sockaddr_ll if_mask;
    memset(&if_mask, 0, sizeof if_mask);

    if_mask.sll_protocol = htons(ETH_P_ALL);
    if_mask.sll_ifindex = if_index;
    if_mask.sll_family = AF_PACKET;

    int ret = bind (socket_fd, &if_mask, sizeof if_mask); 
    if (ret == -1){
        perror("Binding socket to interface");
    }

}

/* Listen for dbus messages from controlling cli */
static void* listen_cli(void *data){
    printf ("Hello\n");
}

