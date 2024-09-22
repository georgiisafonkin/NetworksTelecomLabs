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

typedef struct {
    char ip[INET6_ADDRSTRLEN];
    bool is_active;
}peer_t;

peer_t peers[100];
int peer_count = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int create_socket(int domain, int type, int protocol);
int configure_socket(int socket_fd);
int join_multicast_group(int socket_fd, const char* group_address);

void* peers_listener_func(void* args);
void* peers_speaker_func(void * args);

void handle_error(const char* error_msg);
int is_valid_ipv4_address(const char* address);


int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <multicast_address>\n", argv[0]);
        return EXIT_FAILURE;
    }

    pthread_t listener_tid, speaker_tid;

    pthread_create(&listener_tid, NULL, peers_listener_func, (void*)argv);
    pthread_create(&speaker_tid, NULL, peers_speaker_func, (void*)argv);

    pthread_join(listener_tid, NULL);
    pthread_join(speaker_tid, NULL);

    return EXIT_SUCCESS;
}

int create_socket(int domain, int type, int protocol) {
    int socket_fd = socket(domain, type, protocol);
    if (socket_fd == -1) {
        handle_error("can't create a socket");
    }
}

int configure_socket(int socket_fd) {

}

int join_multicast_group(int socket_fd, const char* group_address) {
    struct ipv6_mreq
}

void* peers_listener_func(void* args) {

}

void* peers_speaker_func(void * args) {

}

void handle_error(const char* error_msg) {
    perror(error_msg);
    exit(EXIT_FAILURE);
}

int is_valid_ipv4_address(const char* address) {
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, address, &(sa.sin_addr));
    return result == 1;
}

int is_valid_ipv6_address(const char* address) {
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET6, address, &(sa.sin_addr));
    return result == 1;
}
