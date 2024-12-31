#define MYPORT "8000"  // the port users will be connecting to
#define BACKLOG 10  

#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include "winSockHelper.h"


int handleAcceptConnection(int listenerfd, int* fd_max, fd_set *master) {
    struct sockaddr_storage remoteaddr;
    socklen_t addrlen;
    int clientfd;
    addrlen = sizeof remoteaddr;
    char remoteIP[INET6_ADDRSTRLEN];
    
    clientfd = accept(listenerfd, (struct sockaddr *)&remoteaddr, &addrlen);

    if (clientfd == -1) {
        return -1;
    }
    FD_SET(clientfd, master);

    if((*fd_max) < clientfd) {
        *fd_max = clientfd;
    }

    printf("selectserver: new connection from %s on "
        "socket %d\n",
        inet_ntop(remoteaddr.ss_family,
            get_in_addr((struct sockaddr*)&remoteaddr),
            remoteIP, INET6_ADDRSTRLEN),
        clientfd);
}

void sendHttpResponse(int* fd) {
    char *status = "200 HTTP/1.1 OK\r\n";
    char *content = "Content-Type: text/html\r\n"; 
    char *length = "Content-Length: ";
    char *crlf = "\r\n";


    const char* path = "./index.html";
    FILE *fptr = {0};
    size_t file_size = {0};


    fptr = fopen(path, "rb");

    fseek(fptr, 0L, SEEK_END);
    file_size = (size_t)ftell(fptr);

    char fileContent[1024];

    rewind(fptr);
    fread(fileContent, sizeof(char), file_size, fptr);
    printf("File size: %d Bytes\n", file_size);
    fclose(fptr);
 
 
    int buffSize = strlen(status) + strlen(content) + strlen(length) + 2*strlen(crlf) + (int)file_size;
    int lengthBuffNums = snprintf(NULL, 0,"%d",file_size);
    buffSize += lengthBuffNums;

    char numsBuffer[1024] = {0};
    snprintf(numsBuffer,1024,"%d",file_size);
    // char sendBuffer[288] = {0};
    char sendBuffer[2048] = {0};

    strcat(sendBuffer, status);
    strcat(sendBuffer, content);
    strcat(sendBuffer, length);
    strcat(sendBuffer, numsBuffer);
    strcat(sendBuffer, crlf);
    strcat(sendBuffer, crlf);
    strcat(sendBuffer, fileContent);
    strcat(sendBuffer, crlf);


    printf(sendBuffer);

    send(*fd, sendBuffer, buffSize, 0);
}

int main(int argc, char *argv[])
{
    printf("Main start..\n");
    WSADATA wsaData;
    initWinsock(&wsaData);

    int status;
    int listener;


    
    fd_set master;
    fd_set read_fds;

    listener = get_listening_socket(MYPORT, BACKLOG);

    if (listener == -1) {
        fprintf(stderr, "error getting listening socket\n");
        exit(1);
    }

    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    char buf[1024];    // Buffer for client data

    int i, j, fd_max, nbytes;

    FD_SET(listener, &master);

    fd_max = listener;

    for(;;) {
        read_fds = master; //copy the data
        if(select(fd_max+1,&read_fds,NULL,NULL,NULL) == -1 ) {
            perror("select");
            exit(4);
        }

        for(i = 0; i <= fd_max; i++) {
            if(FD_ISSET(i,&read_fds)) {
                // printf("here %d\n",i);
                if(i == listener) {
                    if(handleAcceptConnection(listener,&fd_max, &master) == -1) {
                        perror("accept");
                    };
                    continue;
                }
                if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0) {
                        // got error or connection closed by client
                    if (nbytes == 0) {
                        // connection closed
                        printf("selectserver: socket %d hung up\n", i);
                    } else {
                        perror("recv");
                    }
                    closesocket(i); // bye!
                    FD_CLR(i, &master);
                    continue;
                }
                // printf("%s\b", buf);

                //parseHttpRequest();
                //TODO: Receive and proccess data
                for(j = 0; j <= fd_max; j++) {
                // send to everyone!
                if (FD_ISSET(j, &master)) {
                    // except the listener and ourselves
                    //j != i
                    if (j != listener) {
                        sendHttpResponse(&j);
                        // if (send(j, buf, nbytes, 0) == -1) {
                        //     perror("send");
                        // }
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

