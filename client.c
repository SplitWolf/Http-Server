#define MYPORT "8000"  // the port users will be connecting to

#include <stdio.h>
#include <winsock2.h>
#include <windows.h>    
#include <ws2tcpip.h>
#include "winSockHelper.h"

int main(int argc, char const *argv[])
{ 
    WSADATA wsaData;
    initWinsock(&wsaData);

    struct addrinfo hints, *ai;
    int status;
    int server;
   

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET6; // AF_INET or AF_INET6 to force version
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo("::1", MYPORT, &hints, &ai)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 2;
    }

    char buffer[25];
    memset(&buffer, 0, sizeof buffer);
    char* msg = "test from client.c\r\n";
    int stat;

    server = socket(ai->ai_family,ai->ai_socktype,ai->ai_protocol);
    stat = connect(server, ai->ai_addr, ai->ai_addrlen);
    freeaddrinfo(ai);

    printf("wait\n");
    Sleep(1000);
    printf("done\n");


    stat = send(server, msg, strlen(msg), 0);
    stat = shutdown(server, SD_SEND);
    if (stat == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(server);
        WSACleanup();
        return 1;
    }
    stat = recv(server, buffer, strlen(msg), 0);
    printf("stat: %d\n", stat);
    
    printf(buffer);

    stat = recv(server, buffer, 0, 0);
    // cleanup
    closesocket(server);
    WSACleanup();
    Sleep(1000);
    return 0;
}