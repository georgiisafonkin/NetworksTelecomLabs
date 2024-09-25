#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>

#define PORT_LEN 5
#define ADDRS_BUF_SIZE 32

typedef struct {
    const char* mc_group_addr;
    const char* speaker_addr;
    const char* port;
} input_struct;

//TODO structure with ip and last time being in lan
//TODO set/array of this struct and funcs to check active peers in lan
//TODO IPv6

int create_socket(int domain, int type, int protocol);
void make_socket_reusable(int socket_fd);
void configure_listener_socket(int socket_fd, struct sockaddr_in *sin, input_struct *in_struct);
void configure_speaker_socket(int socket_fd, struct sockaddr_in *sin, input_struct *in_struct);
void listener_join_multicast_group(int socket_fd, const char* group_address);

void* peers_listener_func(void* args);
void* peers_speaker_func(void * args);

int is_valid_ipv4_addr(const char* address);
int is_valid_ipv6_addr(const char* address);
void get_sock_name(int socket_fd, struct sockaddr_in* addr, socklen_t* addr_len);
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

void speaker_join_multicast_group(int socket_fd, const char* group_address) {
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
//    const char* mc_group_addr = (char*)args;
    input_struct* in_struct = (input_struct*)args;

    char buffer[1024];

    struct sockaddr_in sin;
    int socket_fd = create_socket(AF_INET, SOCK_DGRAM, 0);
    make_socket_reusable(socket_fd);
    configure_listener_socket(socket_fd, &sin, in_struct);
    listener_join_multicast_group(socket_fd, in_struct->mc_group_addr);

    printf("listener finished socket configuration\n");

    socklen_t sinlen = sizeof(sin);
    while(1) {
        printf("listener invoke recvfrom func\n");
        int responseLen = recvfrom(socket_fd, buffer, 1024, 0, (struct sockaddr*)&sin, &sinlen);
        printf("recvfrom finished\n");
        if (responseLen < 0) {
            handle_error("can't receive a message");
        }
        buffer[responseLen] = '\0';

        printf("Received message: %s\n", buffer);

        printf("listener is here\n");
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
    printf("speaker_unique_addr: %s\n", addr_str);


    while(1) {
        const char* msg = addr_str;
        int n_bytes;
        n_bytes = sendto(socket_fd, msg, strlen(msg), 0, (struct sockaddr*)&addr, sizeof(addr));
        if (n_bytes < 0) {
            handle_error("sendto");
        }
        printf("speaker is here!\n");
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

void get_sock_name(int socket_fd, struct sockaddr_in* addr, socklen_t* addr_len) {
    if (getsockname(socket_fd, (struct sockaddr*)addr, addr_len) < 0) {
        handle_error("Can't get name of socket");
    }
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
