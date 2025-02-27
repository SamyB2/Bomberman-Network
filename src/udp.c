#include "../include/udp.h"
#include "../include/format.h"


// function to start the udp server
void start_udp_servers(uint16_t udp_port, uint16_t multicast_port, Server_info* server_info) {
  int udp_socket, multicast_socket;
  struct sockaddr_in6 servaddr;

  udp_socket = socket(PF_INET6, SOCK_DGRAM, 0);
  server_info->sock_udp = udp_socket;
  multicast_socket = socket(PF_INET6, SOCK_DGRAM, 0);
  server_info->sock_mdff = multicast_socket;


  if (udp_socket < 0 || multicast_socket < 0) {
    perror("socket");
    exit(1);
  }

  if (setsockopt(udp_socket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) {
    perror("setsockopt");
    exit(1);
  }

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin6_family = AF_INET6;
  servaddr.sin6_addr = in6addr_any;
  servaddr.sin6_port = htons(udp_port);

  if (bind(udp_socket, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
    perror("bind udp");
  }
 
  memset(&server_info->addr_mdff, 0, sizeof(server_info->addr_mdff));
  server_info->addr_mdff.sin6_family = AF_INET6;
  inet_pton(AF_INET6, MULTICAST_IP, &server_info->addr_mdff.sin6_addr);
  server_info->addr_mdff.sin6_port = htons(multicast_port);
  server_info->addr_mdff.sin6_scope_id = 0;
  //server_info->addr_mdff.sin6_scope_id = if_nametoindex("wlp2s0");
}