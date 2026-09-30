#ifndef SERVER_H
#define SERVER_H
#endif

#include <sys/socket.h>
#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#define TRUE 1
#define FALSE 0
#define MESSAGE_BUFFER_LEN 1600
#define RESPONSE_BUFFER_LEN 1600

typedef int file_desc;
typedef int bool;

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

/* Sockets Specific Functions */
LinuxSocket *lin_create_skt(uint16_t port, uint32_t ip_addr);
int lin_bind(file_desc skt, uint16_port, uint32_t ip_addr);
int lin_listen(file_desc skt, uint32_t port);
int lin_recv_msg(LinuxSocket *skt, Message *msg, int flags);
int lin_accept(LinuxSocket *skt);
int lin_send(file_desc conn, const void *response, size_t length, int flags);

int process_http(file_desc conn);
int read_method(unsigned char *buf, int buf_len, unsigned char *method);
bool validate_method(unsigned char *method, int len);
void attach_headers(Message *msg, const void *response, int opts);
void parse_version(unsigned char *buffer, size_t buf_len, int start, unsigned start *version);
void parse_target(unsigned char *buffer, size_t buf_len, int start, unsigned char *target);

void handle_sigint(int sig);
