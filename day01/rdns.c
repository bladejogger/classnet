#define _GNU_SOURCE
#define _XOPEN_SOURCE

#include <netdb.h>
#include <stdio.h>
#include <string.h>

#include <arpa/inet.h>

#include <sys/socket.h>

int main(int argc, char *argv[])
{
    if(argc != 2) {
        fprintf(stderr, "Usage: %s <IP>\n", argv[0]);
        return 1;
    }

    struct sockaddr_storage sa;

    void *dst;

    if(strchr(argv[1], ':')) {
        sa.ss_family = AF_INET6;
        dst = &((struct sockaddr_in6 *)&sa)->sin6_addr;
    } else {
        sa.ss_family = AF_INET;
        dst = &((struct sockaddr_in *)&sa)->sin_addr;
    }

    int err = inet_pton(sa.ss_family, argv[1], dst);

    if(err < 0) {
        fprintf(stderr, "Could not parse family %d\n", sa.ss_family);
        return 2;
    } else if(err < 1) {
        fprintf(stderr, "Could not parse address %s\n", argv[1]);
        return 2;
    }

    static char hostname[NI_MAXHOST];

    int name_err = getnameinfo((struct sockaddr *)&sa, sizeof(sa), hostname, sizeof(hostname), NULL, 0, 0);
    if(name_err) {
        fprintf(stderr, "Unable to get information: %s\n", gai_strerror(name_err));
        return 3;
    }

    printf("%s\n", hostname);
}
