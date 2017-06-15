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
    if(argc != 3) {
        fprintf(stderr, "%s <IP> <Message>\n", argv[0]);
        return 1;
    }

    // Port numbers are in the range 1-65535, plus null byte
    char port_num[8];
    snprintf(port_num, sizeof(port_num), "%hd", getuid());

    struct addrinfo *results;

    int err = getaddrinfo(argv[1], port_num, NULL, &results);
    if(err != 0) {
        fprintf(stderr, "Could not parse address: %s\n", gai_strerror(err));
        return 2;
    }

    int sd = socket(results->ai_family, SOCK_DGRAM, 0);
    if(sd < 0) {
        perror("Could not create socket");
        freeaddrinfo(results);
        return 3;
    }

    // Socket descriptor, data to send, length of data, optional flags,
    // sockaddr, sizeof(sockaddr)
    ssize_t sent = sendto(sd, argv[2], strlen(argv[2]), 0,
            results->ai_addr, results->ai_addrlen);
    if(sent < 0) {
        perror("Unable to send");
        close(sd);
        freeaddrinfo(results);
        return 4;
    }

    close(sd);
    freeaddrinfo(results);
}
