#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int create_socket(void);
struct sockaddr_in6 create_address(char *ip, int port);
void send_message(int sockfd, struct sockaddr_in6 addr, char *message);
void start_server(char *ip, int port, char *message);
void *start_udp_server(void *arg);