#ifndef FORMAT_H
#define FORMAT_H

#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include "../include/board.h"

u_int16_t header_id_request (int code_req , int id , int eq);
void header_id_decode (uint16_t req , int* code_Req , int* id , int*eq);
uint32_t action_request (int code_req , int id , int eq , int num , int action);
void action_id_decode (uint16_t req , int* num , int* action);
char* tchat_request (int code_req , int id , int eq , size_t size , char* msg);
char* serv_start_request (int code_req , int id , int eq , uint16_t port_udp , 
                uint16_t port_mdiff , char* adrm_diff);
char* all_board_request(int code_req, int id, int eq, int num, Board* board);
char* change_board_request(int code_req, int id, int eq, int num, Board* board);

#endif