#ifndef UDP_H
#define UDP_H
#include "format.h"

#include <arpa/inet.h>
#include <net/if.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#define MULTICAST_IP "ff02::1"

void start_udp_servers(uint16_t udp_port, uint16_t multicast_port,
                         Server_info* server_info) ;
#endif