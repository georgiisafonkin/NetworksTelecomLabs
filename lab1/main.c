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



typedef struct {
    char ip[INET6_ADDRSTRLEN];
    bool is_active;
};

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
    int socket_fd;
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, "12345", &hints, &result) != 0) {
        perror("getaddrinfo");
        pthread_exit(NULL);
    }

    socket_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (socket_fd == -1) {
        perror("socket");
        freeaddrinfo(result);
        pthread_exit(NULL);
    }


}