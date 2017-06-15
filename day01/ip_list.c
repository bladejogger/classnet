#define _GNU_SOURCE

#include <netdb.h>
#include <stdio.h>

#include <arpa/inet.h>

#include <sys/socket.h>
#include <sys/types.h>

int main(int argc, char *argv[])
{
    if(argc != 2) {
        fprintf(stderr, "Usage: %s <host>\n", argv[0]);
        return 1;
    }

    struct addrinfo hints = {0};
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo *results;
    int err = getaddrinfo(argv[1], NULL, &hints, &results);
    if(err) {
        fprintf(stderr, "Could not get addresses: %s\n", gai_strerror(err));
        return 2;
    }

    for(struct addrinfo *p = results; p; p = p->ai_next) {
        char addr[INET6_ADDRSTRLEN];
        void *addr_struct;

        if(p->ai_family == AF_INET) {
            addr_struct = &(((struct sockaddr_in *)p->ai_addr)->sin_addr);
        } else if(p->ai_family == AF_INET6) {
            addr_struct = &(((struct sockaddr_in6 *)p->ai_addr)->sin6_addr);
        }

        inet_ntop(p->ai_family, addr_struct, addr, sizeof(addr));

        printf("%s\n", addr);
    }

    freeaddrinfo(results);
}
