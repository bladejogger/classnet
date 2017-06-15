#define _GNU_SOURCE

#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>

#include <sys/socket.h>
#include <sys/types.h>

int main(int argc, char *argv[])
{
    if(argc != 2) {
        fprintf(stderr, "%s <IP>\n", argv[0]);
        return 1;
    }

    // Port numbers are in the range 1-65535, plus null byte
    char port_num[8];
    snprintf(port_num, sizeof(port_num), "%hu", getuid());

    struct addrinfo *results;
    struct addrinfo hints = {0};
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    int err = getaddrinfo(argv[1], port_num, &hints, &results);
    if(err != 0) {
        fprintf(stderr, "Could not parse address: %s\n", gai_strerror(err));
        return 2;
    }

    int sd = socket(results->ai_family, results->ai_socktype, 0);
    if(sd < 0) {
        perror("Could not create socket");
        freeaddrinfo(results);
        return 3;
    }

    err = bind(sd, results->ai_addr, results->ai_addrlen);
    if(err < 0) {
        perror("Could not create bind");
        close(sd);
        freeaddrinfo(results);
        return 4;
    }

    freeaddrinfo(results);

    for(;;) {
        // Picked an arbitrary size for example purposes
        char buf[256];
        struct sockaddr_storage client;
        socklen_t client_sz = sizeof(client);
        char ip[INET6_ADDRSTRLEN];
        unsigned short port;

        ssize_t received = recvfrom(sd, buf, sizeof(buf), 0,
                (struct sockaddr *)&client, &client_sz);
        if(received < 0) {
            perror("Problem receiving");
        }

        if(received == 256) {
            buf[received-1] = '\0';
        } else {
            buf[received] = '\0';
        }

        if(client.ss_family == AF_INET6) {
            inet_ntop(client.ss_family,
                    &((struct sockaddr_in6 *)&client)->sin6_addr,
                    ip, sizeof(ip));
            port = ntohs(((struct sockaddr_in6 *)&client)->sin6_port);
        } else {
            inet_ntop(client.ss_family,
                    &((struct sockaddr_in *)&client)->sin_addr,
                    ip, sizeof(ip));
            port = ntohs(((struct sockaddr_in *)&client)->sin_port);
        }

        printf("%s:%hu says '%s'\n", ip, port, buf);
    }

}
