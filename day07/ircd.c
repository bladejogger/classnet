#define _GNU_SOURCE

#include <netdb.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#include <arpa/inet.h>

#include <sys/socket.h>
#include <sys/types.h>

void acceptor(int);
int setup_socket(void);
void log_connection(struct sockaddr *);
void remove_at(struct pollfd *monitors, size_t sz, size_t to_remove);

int main(void)
{
    int verbose = LOG_ERR;

    openlog("ircd", LOG_PERROR, LOG_USER);

    setlogmask(LOG_UPTO(verbose));
    int sd = setup_socket();
    if(sd < 0) {
        return sd;
    }

    acceptor(sd);
}

void acceptor(int sd)
{
    // Horrible hardcoding of max connections; do not do this
    struct pollfd conns[24] = {0};
    size_t conns_idx = 0;

    conns[conns_idx].fd = sd;
    conns[conns_idx].events = POLLIN;
    syslog(LOG_INFO, "Registering %d", conns[conns_idx].fd);
    ++conns_idx;

    for(;;) {
        // Picked an arbitrary size for example purposes
        char buf[256];
        struct sockaddr_storage client;
        socklen_t client_sz = sizeof(client);


        int ready = poll(conns, conns_idx, 3000);
        if(ready < 0) {
            syslog(LOG_WARNING, "Could not poll?");
            continue;
        }
        syslog(LOG_DEBUG, "%d sockets have data", ready);

        for(size_t n=0; n < conns_idx; ++n) {
            syslog(LOG_DEBUG, "Checking %d:%hd)", conns[n].fd, conns[n].revents);
            if(conns[n].revents & POLLHUP) {
                syslog(LOG_INFO, "%d hanging up", conns[n].fd);
                close(conns[n].fd);
                remove_at(conns, conns_idx, n);
                --conns_idx;
                continue;
            }
            if(conns[n].revents & POLLNVAL) {
                syslog(LOG_INFO, "%d invalid", conns[n].fd);
                close(conns[n].fd);
                remove_at(conns, conns_idx, n);
                --conns_idx;
                continue;
            }
            if(conns[n].revents & POLLIN) {
                if(n == 0) {
                    int remote = accept(sd, (struct sockaddr *)&client, &client_sz);
                    if(remote < 0) {
                        syslog(LOG_NOTICE, "Could not accept connection: %m");
                        continue;
                    }
                    log_connection((struct sockaddr *)&client);

                    conns[conns_idx].fd = remote;
                    conns[conns_idx].events = POLLIN;
                    syslog(LOG_INFO, "Registering %d", conns[conns_idx].fd);
                    ++conns_idx;
                } else {
                    ssize_t received = recv(conns[n].fd, buf, sizeof(buf)-1, 0);
                    if(received < 0) {
                        syslog(LOG_NOTICE, "Could not receive data: %m");
                        continue;
                    } else if(received == 0) {
                        syslog(LOG_NOTICE, "%d end of recv", conns[n].fd);
                        close(conns[n].fd);
                        conns[n].events = 0;
                        remove_at(conns, conns_idx, n);
                        --conns_idx;
                    } else {
                        buf[received] = '\0';
                        for(size_t m=1; m < conns_idx; ++m) {
                            if(m == n) {
                                continue;
                            }
                            send(conns[m].fd, buf, received, 0);
                        }
                    }
                }
            }
        }

    }
}

void remove_at(struct pollfd *monitors, size_t sz, size_t to_remove)
{
    memmove(monitors + to_remove, monitors + to_remove + 1, sizeof(*monitors)*(sz - to_remove - 1));
}

void log_connection(struct sockaddr *client)
{
    char ip[INET6_ADDRSTRLEN];
    unsigned short port;

    if(client->sa_family == AF_INET6) {
        inet_ntop(client->sa_family,
                &((struct sockaddr_in6 *)&client)->sin6_addr,
                ip, sizeof(ip));
        port = ntohs(((struct sockaddr_in6 *)&client)->sin6_port);
    } else {
        inet_ntop(client->sa_family,
                &((struct sockaddr_in *)&client)->sin_addr,
                ip, sizeof(ip));
        port = ntohs(((struct sockaddr_in *)&client)->sin_port);
    }
    syslog(LOG_INFO, "%s:%hu is connected", ip, port);

}

int setup_socket(void)
{
    // Port numbers are in the range 1-65535, plus null byte
    char port_num[8];
    snprintf(port_num, sizeof(port_num), "%hu", getuid());

    struct addrinfo *results;
    struct addrinfo hints = {0};
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int err = getaddrinfo(NULL, port_num, &hints, &results);
    if(err != 0) {
        fprintf(stderr, "Could not parse address: %s\n", gai_strerror(err));
        return -2;
    }

    int sd = socket(results->ai_family, results->ai_socktype, 0);
    if(sd < 0) {
        perror("Could not create socket");
        freeaddrinfo(results);
        return -3;
    }

    int set = 1;
    err = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &set, sizeof(set));
    if(err < 0) {
        perror("Could not bind");
        close(sd);
        freeaddrinfo(results);
        return -4;
    }

    err = bind(sd, results->ai_addr, results->ai_addrlen);
    if(err < 0) {
        perror("Could not bind");
        close(sd);
        freeaddrinfo(results);
        return -5;
    }
    freeaddrinfo(results);

    // 5 is the usual backlog
    err = listen(sd, 5);
    if(err < 0) {
        perror("Could not listen");
        return -6;
    }

    return sd;
}
