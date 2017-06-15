
#include <ifaddrs.h>
#include <stdint.h>
#include <stdio.h>

#include <arpa/inet.h>

#include <linux/if_packet.h>

#include <sys/types.h>
#include <sys/un.h>

int main(void)
{
    struct ifaddrs *results;

    int err = getifaddrs(&results);
    if(err) {
        perror("Could not get list");
        return 1;
    }

    for(struct ifaddrs *p = results; p; p = p->ifa_next) {
        char addr[INET6_ADDRSTRLEN];
        
        struct sockaddr *sa = p->ifa_addr;
        if(sa->sa_family == AF_UNIX) {
            printf("UNIX %s\n", ((struct sockaddr_un *)sa)->sun_path);
        } else if(sa->sa_family == AF_PACKET) {
            uint8_t *ptr = (uint8_t *)((struct sockaddr_ll *)sa)->sll_addr;
            printf("LINK %02x:%02x:%02x:%02x:%02x:%02x\n", ptr[0],ptr[1],ptr[2],ptr[3],ptr[4],ptr[5]);
        } else if(sa->sa_family == AF_INET) {
            inet_ntop(sa->sa_family, &((struct sockaddr_in *)sa)->sin_addr, addr, sizeof(addr));
            printf("IPv4: %s\n", addr);
        } else if(sa->sa_family == AF_INET6) {
            inet_ntop(sa->sa_family, &((struct sockaddr_in6 *)sa)->sin6_addr, addr, sizeof(addr));
            printf("IPv6: %s\n", addr);
        }
    }


    freeifaddrs(results);
}
