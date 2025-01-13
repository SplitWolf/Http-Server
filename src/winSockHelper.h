#include <winsock2.h>
int initWinsock(WSADATA *wsaData);
void *get_in_addr(struct sockaddr *sa);
int get_listening_socket(const char* port, int backlog);
void add_to_pfds(struct pollfd *pfds[], int newfd, int *fd_count, int *fd_size);
void del_from_pfds(struct pollfd pfds[], int i, int *fd_count);
