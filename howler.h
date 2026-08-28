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
    file_desc fc;
    uint16_t  port;
    uint32_t  ip_addr;
} LinuxSocket;

enum StatusCode {
    STATUSOK = 200,
};

void get();
void put();
void del();
void post();

int lin_bind(file_desc skt, uint16_port, uint32_t ip_addr);
int lin_listen(file_desc skt, uint32_t port);