#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>

#define PORT_LEN 5
#define PEERS_N 32
#define LIVE_TIMOUT 60
#define CHECK_TIMEOUT 60
#define WRITE_TIMEOUT 5
#define TIME_COEF 100000

typedef struct {
    const char* mc_group_addr;
    const char* speaker_addr;
    const char* port;
} input_struct;

typedef struct {
    char ip_addr[INET_ADDRSTRLEN];
    double last_time_was_active;
}peer_t;

typedef struct {
    peer_t peers[PEERS_N];
    int length;
    int capacity;
}peers_list;


pthread_mutex_t mutex;

//TODO IPv6 support
//TODO not count self in actives list

peers_list list;
clock_t last_time_checked;
clock_t last_time_written;
pthread_mutex_t mutex;

int is_alive(peer_t peer);
int remove_peer_at(int i, peers_list* list);
void remove_dead_peers(peers_list *list);
void update_list_with_peer(peer_t peer, peers_list* list);
void print_actives(peers_list* list);

int create_socket(int domain, int type, int protocol);
void make_socket_reusable(int socket_fd);
void configure_listener_socket(int socket_fd, struct sockaddr_in *sin, input_struct *in_struct);
void configure_speaker_socket(int socket_fd, struct sockaddr_in *sin, input_struct *in_struct);
void listener_join_multicast_group(int socket_fd, const char* group_address);

void* peers_listener_func(void* args);
void* peers_speaker_func(void * args);

int is_valid_ipv4_addr(const char* address);
int is_valid_ipv6_addr(const char* address);
int get_addr_as_str(struct sockaddr_in addr, char* str_buffer, size_t buff_len);

void handle_error(const char* error_msg);

int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <multicast_address> <speaker_address> <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    input_struct in;
    in.mc_group_addr = argv[1];
    in.speaker_addr = argv[2];
    in.port = argv[3];

    pthread_t listener_tid, speaker_tid;

    pthread_create(&listener_tid, NULL, peers_listener_func, (void*)&in);
    pthread_create(&speaker_tid, NULL, peers_speaker_func, (void*)&in);

    pthread_join(listener_tid, NULL);
    pthread_join(speaker_tid, NULL);

    return EXIT_SUCCESS;
}

int is_alive(peer_t peer) {
//    printf("time since last active time: %lf\n", ((double)(clock() - peer.last_time_was_active))/CLOCKS_PER_SEC);
//    printf("live_timeout/time_coef: %lf\n", ((double)LIVE_TIMOUT/TIME_COEF));
    pthread_mutex_lock(&mutex);
    if (((double)(clock() - peer.last_time_was_active))/CLOCKS_PER_SEC > (double)LIVE_TIMOUT/TIME_COEF) {
        printf("peer %s is dead\n", peer.ip_addr);
        return 0;
    }
//    printf("peer is not dead\n");
    pthread_mutex_unlock(&mutex);
    return 1;
}

int remove_peer_at(int i, peers_list* list) {
    int distance = list->length - i;
    if (distance < 0) {
        handle_error("remove_peer_at: incorrect index");
    }
    memmove(&list->peers + i, &list->peers + i + 1, distance * sizeof(peer_t));
    bzero(&list->peers + list->length - 1, 1);
    --list->length;
//    printf("list's length decremented\n");
}

void remove_dead_peers(peers_list *list) {
    if (((double)(clock() - last_time_checked))/CLOCKS_PER_SEC <= (double)CHECK_TIMEOUT/TIME_COEF) {
        return;
    }
    for(int i = 0; i < list->length; ++i) {
        if (!is_alive(list->peers[i])) {
            remove_peer_at(i, list);
        }
    }
    last_time_checked = clock();
}

void update_list_with_peer(peer_t peer, peers_list* list) {
    int is_here_already = 0;

    for(int i = 0; i < list->length; ++i) {
        if (strcmp(peer.ip_addr, list->peers[i].ip_addr) == 0) {
//            printf("I SEE SAME ADDRS:\n");
//            printf("checking peer: %s\n", peer.ip_addr);
//            printf("peer at list with i = %d: %s\n", i, list->peers[i].ip_addr);
            is_here_already = 1;
            list->peers[i].last_time_was_active = peer.last_time_was_active;
        }
    }

//    printf("is_here_already: %d\n", is_here_already);

    if (!is_here_already) {
//        printf("LENGTH BEFORE ADDING: %d\n", list->length);
//        printf("list's length incremented\n");
        strcpy(list->peers[list->length].ip_addr, peer.ip_addr);
//        list->peers[list->length].ip_addr = peer.ip_addr;
        list->peers[list->length].last_time_was_active = peer.last_time_was_active;
        ++list->length;
//        printf("LENGTH BEFORE ADDING: %d\n", list->length);
    }
}

void print_actives(peers_list* list) {
    if (((double)(clock() - last_time_written))/CLOCKS_PER_SEC <= (double)WRITE_TIMEOUT/TIME_COEF) {
        return;
    }
    printf("%d active copies:\n", list->length);
    for (int i = 0; i < list->length; ++i) {
        printf("%s\n", list->peers[i].ip_addr);
    }
    last_time_written = clock();
}

int create_socket(int domain, int type, int protocol) {
    int socket_fd = socket(domain, type, protocol);
    if (socket_fd == -1) {
        handle_error("can't create a socket");
    }
}

void configure_listener_socket(int socket_fd, struct sockaddr_in *sin, input_struct *in_struct) {
    bzero(sin, sizeof(*sin));
    sin->sin_family = AF_INET;
    sin->sin_addr.s_addr = htonl(INADDR_ANY);
    sin->sin_port = htons(atoi(in_struct->port)); //TODO

    if (bind(socket_fd, (struct sockaddr*)sin, sizeof(*sin)) < 0) {
        perror("bind failed");
        exit(1);
    }
}

void make_socket_reusable(int socket_fd) {
    u_int yes = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (char*) &yes, sizeof(yes)) < 0){
        handle_error("can't make socket reusable");
    }
}

void configure_speaker_socket(int socket_fd, struct sockaddr_in *sin, input_struct *in_struct) {
    bzero(sin, sizeof(*sin));
    sin->sin_family = AF_INET;
    sin->sin_addr.s_addr = inet_addr(in_struct->mc_group_addr);
    sin->sin_port = htons(atoi(in_struct->port));

//    if (bind(socket_fd, (struct sockaddr*)sin, sizeof(*sin)) < 0) {
//        handle_error("can't bind speaker:");
//    }
}

void listener_join_multicast_group(int socket_fd, const char* group_address) {
    if (is_valid_ipv4_addr(group_address)) {
        struct ip_mreq mreq;

        if (inet_pton(AF_INET, group_address, &(mreq.imr_multiaddr.s_addr)) <= 0) {
            handle_error("inet_pton");
        }
//        mreq.imr_multiaddr.s_addr = inet_addr(group_address);
        mreq.imr_interface.s_addr = htonl(INADDR_ANY);

        if (setsockopt(socket_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq)) < 0) {
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
    last_time_checked = clock();
    last_time_written = clock();
    input_struct* in_struct = (input_struct*)args;

    char buffer[1024];

    struct sockaddr_in sin;
    int socket_fd = create_socket(AF_INET, SOCK_DGRAM, 0);
    make_socket_reusable(socket_fd);
    configure_listener_socket(socket_fd, &sin, in_struct);
    listener_join_multicast_group(socket_fd, in_struct->mc_group_addr);

//    printf("listener finished socket configuration\n");

    socklen_t sinlen = sizeof(sin);

    list.length = 0;
    list.capacity = PEERS_N;

    while(1) {
        remove_dead_peers(&list);
//        printf("listener invoke recvfrom func\n");
        int responseLen = recvfrom(socket_fd, buffer, 1024, 0, (struct sockaddr*)&sin, &sinlen);
//        printf("recvfrom finished\n");
        if (responseLen < 0) {
            handle_error("can't receive a message");
        }
        buffer[responseLen] = '\0';

        peer_t listened_peer;
        strcpy(listened_peer.ip_addr, buffer);
//        listened_peer.ip_addr = buffer;
        listened_peer.last_time_was_active = clock();

        update_list_with_peer(listened_peer, &list);

        print_actives(&list);
//        printf("Received message: %s\n", buffer);

//        printf("listener is here\n");
//        sleep(5);
    }
}

void* peers_speaker_func(void * args) {
//    const char* mc_group_addr = (char*)args;
    input_struct* in_struct = (input_struct*)args;

    char buffer[1024];

    struct sockaddr_in addr;
    int socket_fd = create_socket(AF_INET, SOCK_DGRAM, 0);
    make_socket_reusable(socket_fd);
    configure_speaker_socket(socket_fd, &addr, in_struct); //TODO rename func to configure send socket
//    speaker_join_multicast_group(socket_fd, in_struct);

//    socklen_t sinlen = sizeof(addr);
    struct sockaddr_in unique_addr;
    unique_addr.sin_family = AF_INET;
    unique_addr.sin_addr.s_addr = inet_addr(in_struct->speaker_addr);
    unique_addr.sin_port = htons(atoi(in_struct->port));
    if (bind(socket_fd, (struct sockaddr*)&unique_addr, sizeof(unique_addr)) < 0) {
        handle_error("can't bind socket to unique addr");
    }
//    get_sock_name(socket_fd, &unique_addr, &sinlen);
    char addr_str[INET_ADDRSTRLEN + PORT_LEN + 1];
    get_addr_as_str(unique_addr, addr_str, sizeof(addr_str));
//    printf("speaker_unique_addr: %s\n", addr_str);


    while(1) {
        const char* msg = addr_str;
        int n_bytes;
        n_bytes = sendto(socket_fd, msg, strlen(msg), 0, (struct sockaddr*)&addr, sizeof(addr));
        if (n_bytes < 0) {
            handle_error("sendto");
        }
//        printf("speaker is here!\n");
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

int get_addr_as_str(struct sockaddr_in addr, char* str_buffer, size_t buff_len) {
    //2 = ':' + '\0'
    if (buff_len < INET_ADDRSTRLEN + PORT_LEN + 1) {
        return -1;
    }

    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr.sin_addr, ip_str, sizeof(ip_str));
    snprintf(str_buffer, buff_len, "%s:%d", ip_str, ntohs(addr.sin_port));
    //strBuf[INET_ADDRSTRLEN + PORT_LEN + 1] = '\0';
    return 0;
}
