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

#define BUFFER_SIZE 64
#define LISTENER_PORT 4040

typedef struct {
    char ip[INET6_ADDRSTRLEN];
    bool is_active;
}peer_t;

peer_t peers[100];
int peer_count = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int create_socket(int domain, int type, int protocol);
int configure_listener_socket(int socket_fd, const char* mc_group_addr);
int join_multicast_group(int socket_fd, const char* group_address);

void* peers_listener_func(void* args);
void* peers_speaker_func(void * args);

void handle_error(const char* error_msg);
int is_valid_ipv4_addr(const char* address);
int is_valid_ipv6_addr(const char* address);

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <multicast_address>\n", argv[0]);
        return EXIT_FAILURE;
    }

    pthread_t listener_tid, speaker_tid;

//    pthread_create(&listener_tid, NULL, peers_listener_func, (void*)argv[1]);
//    pthread_create(&speaker_tid, NULL, peers_speaker_func, (void*)argv[1]);

//    pthread_join(listener_tid, NULL);
//    pthread_join(speaker_tid, NULL);

    //TEST, will be deleted or replaced later
    const char* mc_group_addr = argv[1];
    char buffer[1024];

    int socket_fd = create_socket(AF_INET, SOCK_DGRAM, 0);
    configure_listener_socket(socket_fd, mc_group_addr);
    join_multicast_group(socket_fd, mc_group_addr);

    while(1) {
        int responseLen = recvfrom(socket_fd, buffer, 1024, 0, NULL, NULL);
        if (responseLen < 0) {
            handle_error("can't receive a message");
        }
        buffer[responseLen] = '\0';

        printf("Received message: %s\n", buffer);

        sleep(5);
    }

    return EXIT_SUCCESS;
}

int create_socket(int domain, int type, int protocol) {
    int socket_fd = socket(domain, type, protocol);
    if (socket_fd == -1) {
        handle_error("can't create a socket");
    }
}

int configure_listener_socket(int socket_fd, const char* mc_group_addr) {
    struct sockaddr_in sin;

    bzero(&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(LISTENER_PORT);

    if (bind(socket_fd, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        perror("bind failed");
        exit(1);
    }

//    if (inet_pton(AF_INET, mc_group_addr, &sin.sin_addr) <= 0) {
//        handle_error("Invalid address");
//    }
//    if (bind(socket_fd, (struct sockaddr*)&sin, sizeof(sin)) != 0) {
//        handle_error("Can't bind socket");
//    }

    //make socket reusable
    const int optVal = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal)) != 0 ||
        setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &optVal, sizeof(optVal)) != 0) {
        handle_error("Can't set socket options");
    }
}

int join_multicast_group(int socket_fd, const char* group_address) {
    if (is_valid_ipv4_addr(group_address)) {
        struct ip_mreq mreq;

        if (inet_pton(AF_INET, group_address, &(mreq.imr_multiaddr.s_addr)) != 1) {
            handle_error("inet_pton");
        }
        mreq.imr_interface.s_addr = htonl(INADDR_ANY);

        if (setsockopt(socket_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
            handle_error("can't join multicast group");
        }
    } else if (is_valid_ipv6_addr(group_address)) {
        struct ipv6_mreq mreq6;

        if (inet_pton(AF_INET, group_address, &(mreq6.ipv6mr_multiaddr)) != 1) {
            handle_error("inet_pton");
        }

        if (setsockopt(socket_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq6, sizeof(mreq6)) != 0) {
            handle_error("can't join multicast group");
        }
    } else {
        handle_error("invalid ip address");
    }
}

void* peers_listener_func(void* args) {
    const char* mc_group_addr = (char*)args;

    char buffer[1024];

    int socket_fd = create_socket(AF_INET, SOCK_DGRAM, 0);
    configure_listener_socket(socket_fd, mc_group_addr);
    join_multicast_group(socket_fd, mc_group_addr);

    while(1) {
        int responseLen = recvfrom(socket_fd, buffer, 1024, 0, NULL, NULL);
        if (responseLen < 0) {
            handle_error("can't receive a message");
        }
        buffer[responseLen] = '\0';

        printf("Received message: %s\n", buffer);

        sleep(5);
    }
}

void* peers_speaker_func(void * args) {
    const char* mc_group_addr = (char*)args;

    char buffer[1024];

    int socket_fd = create_socket(AF_INET, SOCK_DGRAM, 0);
    configure_listener_socket(socket_fd, mc_group_addr);
    join_multicast_group(socket_fd, mc_group_addr);

    while(1) {
        const char* msg = "Hello from multicast group!\n";
        sendto(socket_fd, msg, strlen(msg), 0, NULL, 0);

        sleep(5);
    }
}

void handle_error(const char* error_msg) {
    perror(error_msg);
    exit(EXIT_FAILURE);
}

int is_valid_ipv4_addr(const char* address) {
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, address, &(sa.sin_addr));
    return result == 1;
}

int is_valid_ipv6_addr(const char* address) {
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET6, address, &(sa.sin_addr));
    return result == 1;
}
