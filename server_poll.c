#define MYPORT "8000"  // the port users will be connecting to
#define BACKLOG 10  

#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include "winSockHelper.h"
#include <fcntl.h>

int main(int argc, char *argv[])
{
    WSADATA wsaData;
    initWinsock(&wsaData);

    int status;
    int listener, clientSocket;

    listener = get_listening_socket(MYPORT, BACKLOG);

    if (listener == -1) {
        fprintf(stderr, "error getting listening socket\n");
        exit(1);
    }

    struct sockaddr_storage remoteaddr;
    socklen_t addrlen;

    char buf[256];    // Buffer for client data
    char remoteIP[INET6_ADDRSTRLEN];

    ioctlsocket(listener, FIONBIO, 0);


    int fd_count = 0;
    int fd_size = 5;
    struct pollfd *pfds = malloc(sizeof *pfds * fd_size);

    pfds[0].fd = listener;
    pfds[0].events = POLLIN; // Report ready to read on incoming connection

    fd_count = 1;

    while(1) {
        int poll_count = WSAPoll(pfds, fd_count, -1);

        if (poll_count == -1) {
            perror("poll");
            exit(1);
        }

        for(int i = 0; i < fd_count; i++) {

            if(pfds[i].revents & POLLIN) {
                if(pfds[i].fd == listener) {
                    addrlen = sizeof remoteaddr;
                    clientSocket = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);
                        if (clientSocket == -1) {
                        perror("accept");
                    } else {
                        add_to_pfds(&pfds, clientSocket, &fd_count, &fd_size);

                        printf("pollserver: new connection from %s on "
                            "socket %d\n",
                            inet_ntop(remoteaddr.ss_family,
                                get_in_addr((struct sockaddr*)&remoteaddr),
                                remoteIP, INET6_ADDRSTRLEN),
                            clientSocket);
                    }
                } else {
                    int nbytes = recv(pfds[i].fd, buf, sizeof buf, 0);

                    int sender_fd = pfds[i].fd;

                    printf("n bytes: %d\n",nbytes);
                    if (nbytes <= 0) {
                        printf("Not here");
                        // Got error or connection closed by client
                        if (nbytes == 0) {
                            // Connection closed
                            printf("pollserver: socket %d hung up\n", sender_fd);
                        } else {
                            perror("recv");
                        }

                        closesocket(pfds[i].fd); // Bye!

                        del_from_pfds(pfds, i, &fd_count);

                    } else {
                        // We got some good data from a client
                        printf("%d\n", fd_count);

                        for(int j = 0; j < fd_count; j++) {
                            // Send to everyone!
                            int dest_fd = pfds[j].fd;

                            // Except the listener and ourselves
                            //&& dest_fd != sender_fd
                            if (dest_fd != listener) {
                                if (send(dest_fd, buf, nbytes, 0) == -1) {
                                    perror("send\n");
                                    printf("%d\n", WSAGetLastError());
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    WSACleanup();
    closesocket(listener);

    return 0;
}

