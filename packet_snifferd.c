#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <errno.h>

#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h> 

#define MAX_FRAME_SIZE 55555

/* Name of the interface sniffed on */
static char *if_name;

/* Packet sniffer daemon. */
int main(void){

    int socket_fd;

    socket_fd = socket(AF_PACKET, SOCK_DGRAM, htons(ETH_P_ALL));    
    if (socket_fd == -1){
        perror ("PACKET socket creation");
        fprintf (stderr, "Ensure daemon is run with superuser privileges\n");
        return -1;
    }

    struct sockaddr_ll if_mask;
    memset(&if_mask, 0, sizeof if_mask);
    if_mask.sll_protocol = htons(ETH_P_ALL);
    if_mask.sll_ifindex = 2;
    if_mask.sll_family = AF_PACKET;
    int ret = bind (socket_fd, &if_mask, sizeof if_mask); 
    if (ret == -1){
        printf ("Error %d\n", errno);
        perror("Wrong smth");
    }


    memset(&if_mask, 0, sizeof if_mask);
    if_mask.sll_protocol = htons(ETH_P_ALL);
    if_mask.sll_ifindex = 3;
    if_mask.sll_family = AF_PACKET;
    ret = bind (socket_fd, &if_mask, sizeof if_mask); 
    if (ret == -1){
        printf ("insecond\n");
        printf ("Error %d\n", errno);
        perror("Wrong smth");
    }


    char frame_buffer[MAX_FRAME_SIZE];
    while (1) {
        int received_bytes = 
            recv(socket_fd, frame_buffer, sizeof(frame_buffer), 0);
        printf ("After buffer filled with data, size: %d\n", received_bytes);
        struct iphdr *header = (struct iphdr *) frame_buffer;

        struct in_addr source_addr, dest_addr;
        source_addr.s_addr = header->saddr;
        dest_addr.s_addr = header->daddr;
        printf ("Packet source addr: %s\n", inet_ntoa(source_addr));
        printf ("Packet dest addr: %s\n", inet_ntoa(dest_addr));
    }

    return 0;

}
