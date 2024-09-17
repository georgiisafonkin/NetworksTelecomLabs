#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <error.h>
#include <stdbool.h>


typedef struct {
    char ip[INET6_ADDRSTRLEN];
    bool is_active;
}peer_t;

peer_t peers[100];
int peer_count = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* peers_listener_func(void* args);
void* peers_communicator_func(void * args);

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <multicast_address>\n", argv[0]);
    }
    return EXIT_SUCCESS;
}

void* peers_listener_func(void* args) {
    char* multicast_group_address = (char*)args;
    int socket_fd;
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    //receiving local area network addresses
    if (getaddrinfo(NULL, "12345", &hints, &result) != 0) {
        perror("getaddrinfo");
        pthread_exit(NULL);
    }

    //receiving socket file descriptor
    socket_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (socket_fd == -1) {
        perror("socket");
        freeaddrinfo(result);
        pthread_exit(NULL);
    }

    //connecting to multicast group
    if (result->ai_family == AF_INET) {
        struct ip_mreq ipv4_mreq;
        //assigning multicast-group address in byte as binary data in network byte order
        ipv4_mreq.imr_multiaddr.s_addr = inet_addr(multicast_group_address); //TO DO it with aton later, because it more modern and safe
        //idk why it is here
        ipv4_mreq.imr_interface.s_addr = htonl(INADDR_ANY);
        //set options from "mreq" structure to our socket
        setsockopt(socket_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &ipv4_mreq, sizeof(ipv4_mreq));
    } else if (result->ai_family == AF_INET6) {
        struct ipv6_mreq ipv6_mreq;
        inet_pton(AF_INET6, multicast_group_address, &ipv6_mreq.ipv6mr_multiaddr);
        ipv6_mreq.ipv6mr_interface = 0; //default interface
        setsockopt(socket_fd, IPPROTO_IPV6, IPV6_JOIN_GROUP, &ipv6_mreq, sizeof(ipv6_mreq));
    }

    bind(socket_fd, result->ai_addr, result->ai_addrlen);
    freeaddrinfo(result);

    while(true) {

    }
}