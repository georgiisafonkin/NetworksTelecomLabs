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

#define BUFFER_SIZE 64

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
    char buffer[BUFFER_SIZE];
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
        ipv4_mreq.imr_multiaddr.s_addr = inet_addr(multicast_group_address); //TODO it with aton later, because it more modern and safe
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

    //listening...
    while (1) {
        socklen_t addr_len = sizeof(struct sockaddr_storage);
        struct sockaddr_storage addr;
        int n = recvfrom(socket_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &addr, &addr_len);
        if (n < 0) {
            perror("recvfrom");
            continue;
        }
        buffer[n] = '\0';

        char ip[INET6_ADDRSTRLEN];
        if (addr.ss_family == AF_INET) {
            struct sockaddr_in *s = (struct sockaddr_in *) &addr;
            inet_ntop(AF_INET, &s->sin_addr, ip, sizeof(ip));
        } else {
            struct sockaddr_in6 *s = (struct sockaddr_in6 *) &addr;
            inet_ntop(AF_INET6, &s->sin6_addr, ip, sizeof(ip));
        }

        //updating list of peers
        pthread_mutex_lock(&mutex);

        int found = 0;

        for (int i = 0; i < peer_count; ++i) {
            if (strcmp(peers[i].ip, ip) == 0) {
                peers[i].is_active = true;
                found = 1;
                break;
            }
        }

        if (!found && peer_count < 100) {
            strcpy(peers[peer_count].ip, ip);
            peers[peer_count].is_active = true;
            ++peer_count;
        }

        pthread_mutex_unlock(&mutex);

        //output the list of active peers
        printf("active copies:\n");
        pthread_mutex_lock(&mutex);
        for (int i = 0; i < peer_count; ++i) {
            if (peers[i].is_active) {
                printf("%s\n", peers[i].ip);
            }
        }
        pthread_mutex_unlock(&mutex);
    }

    close(socket_fd);
    pthread_exit(NULL);
}
