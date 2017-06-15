
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

    struct sockaddr *endpoint;
    socklen_t endpoint_sz;
    int sd;

    if(strchr(argv[1], ':')) {
        struct sockaddr_in6 dest = {0};
        dest.sin6_family = AF_INET6;
        dest.sin6_port = htons(getuid());
        int err = inet_pton(dest.sin6_family, argv[1], &dest.sin6_addr);
        if(err < 0) {
            fprintf(stderr, "Could not parse family %d\n", dest.sin6_family);
            return 2;
        } else if(err < 1) {
            fprintf(stderr, "Could not parse address %s\n", argv[1]);
            return 2;
        }
        endpoint = &dest;
        endpoint_sz = sizeof(dest);

        sd = socket(PF_INET6, SOCK_DGRAM, 0);
        if(sd < 0) {
            perror("Could not create socket");
            return 3;
        }

    } else {
        struct sockaddr_in dest = {0};
        dest.sin_family = AF_INET;
        dest.sin_port = htons(getuid());
        int err = inet_pton(dest.sin_family, argv[1], &dest.sin_addr);
        if(err < 0) {
            fprintf(stderr, "Could not parse family %d\n", dest.sin_family);
            return 2;
        } else if(err < 1) {
            fprintf(stderr, "Could not parse address %s\n", argv[1]);
            return 2;
        }
        endpoint = &dest;
        endpoint_sz = sizeof(dest);

        sd = socket(PF_INET, SOCK_DGRAM, 0);
        if(sd < 0) {
            perror("Could not create socket");
            return 3;
        }
    }


    // Socket descriptor, data to send, length of data, optional flags,
    // sockaddr, sizeof(sockaddr)
    ssize_t sent = sendto(sd, argv[2], strlen(argv[2]), 0,
            endpoint, endpoint_sz);
    if(sent < 0) {
        perror("Unable to send");
        close(sd);
        return 4;
    }

    close(sd);
}
