#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include <arpa/inet.h>
#include<stdbool.h>

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

}