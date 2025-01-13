#include <stdio.h>
#include "winSockHelper.h"
#include <ws2tcpip.h>


void *get_in_addr(struct sockaddr *sa)
{
      if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int get_listening_socket(const char* port, int backlog) {
    int listener;
    char yes='1';
    int retVal;

    struct addrinfo hints, *res, *p;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((retVal = getaddrinfo(NULL, port, &hints, &res)) != 0) {
        fprintf(stderr, "pollserver: %s\n", gai_strerror(retVal));
        exit(1);
    }

    for(p = res; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family,p->ai_socktype, p->ai_protocol);
        if (listener < 0) { 
            continue;
        }
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            closesocket(listener);
            continue;
        }

        break;
    }

    freeaddrinfo(res);

    if(p == NULL) return -1;


    // Listen
    if (listen(listener, backlog) == -1) {
        return -1;
    }

    return listener;
}

void add_to_pfds(struct pollfd *pfds[], int newfd, int *fd_count, int *fd_size) {
    if(*fd_count == * fd_size) {
        *fd_size *= 2;
        *pfds = realloc(*pfds, sizeof(**pfds) * (*fd_size));
    }

    (*pfds)[*fd_count].fd = newfd;
    (*pfds)[*fd_count].events = POLLIN;

    (*fd_count)++;

}

void del_from_pfds(struct pollfd pfds[], int i, int *fd_count)
{
    // Copy the one from the end over this one
    pfds[i] = pfds[*fd_count-1];

    (*fd_count)--;
}

int initWinsock(WSADATA *wsaData) {
    if (WSAStartup(MAKEWORD(2, 2), wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed.\n");
        exit(1);
    }

    if (LOBYTE(wsaData->wVersion) != 2 ||
        HIBYTE(wsaData->wVersion) != 2)
    {
        fprintf(stderr,"Versiion 2.2 of Winsock is not available.\n");
        WSACleanup();
        exit(2);
    }
}