#define _GNU_SOURCE

#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>

#include <sys/socket.h>
#include <sys/types.h>

// The next 39 lines come from the day02/udp_server
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
    hints.ai_socktype = SOCK_STREAM;

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
        perror("Could not bind");
        close(sd);
        freeaddrinfo(results);
        return 4;
    }
    freeaddrinfo(results);

    // 5 is the usual backlog
    err = listen(sd, 5);
    if(err < 0) {
        perror("Could not listen");
        freeaddrinfo(results);
        return 4;
    }

    // Next 36 lines also pulled from say02/udp_server
    for(;;) {
        // Picked an arbitrary size for example purposes
        char buf[256];
        struct sockaddr_storage client;
        socklen_t client_sz = sizeof(client);
        char ip[INET6_ADDRSTRLEN];
        unsigned short port;

        int client_descriptor = accept(sd, (struct sockaddr *)&client, &client_sz);

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

        printf("%s:%hu is connected\n", ip, port);

        ssize_t received = recv(client_descriptor, buf, sizeof(buf)-1, 0);

        while(received > 0) {
            buf[received] = '\0';
            printf("%s", buf);
            received = recv(client_descriptor, buf, sizeof(buf)-1, 0);
        }

        if(received < 0) {
            perror("Problem receiving");
        }


        printf("%s:%hu says '%s'\n", ip, port, buf);
    }

}
