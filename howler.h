#ifndef SERVER_H
#define SERVER_H
#endif

#include <sys/socket.h>
#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <errno.h>

typedef int file_desc

typedef struct LinuxSocket {
    file_desc fd;
    
    uint16_t  port;
    uint32_t  ip_addr;
} LinuxSocket;

typedef struct Message {
    int buf_len;
    void *buffer;
} Message; 

enum StatusCode {
    STATUSOK = 200,
};

void get();
void put();
void del();
void post();

LinuxSocket *lin_create_skt(uint16_t port, uint32_t ip_addr);
int lin_bind(file_desc skt, uint16_port, uint32_t ip_addr);
int lin_listen(file_desc skt, uint32_t port);
int lin_recv_msg(LinuxSocket *skt, Message *msg, int flags);
