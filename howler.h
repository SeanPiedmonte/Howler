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

enum VERSION {
    HTTP1_1 = 1,
    HTTP2   = 2,
    HTTP3   = 3,
};

typedef struct LinuxSocket {
    file_desc fd;
    
    uint16_t  port;
    uint32_t  ip_addr;
} LinuxSocket;

typedef struct Message {
    int buf_len;
    void *buffer;
} Message; 

typedef struct Request {
    unsigned char *method;
    unsigned char *target;
    unsigned char *host;
    unsigned char *user_agent;
    unsigned char *accept;
    VERSION       version;
    int           port;
} Request;

enum StatusCode {
    STATUSOK = 200,
};

void get();
void put();
void del();
void post();

/* Sockets Specific Functions */
LinuxSocket *lin_create_skt(uint16_t port, uint32_t ip_addr);
int lin_bind(LinuxSocket *skt);
int lin_listen(LinuxSocket *skt, int backlog);
int lin_recv_msg(file_desc conn, Message *msg, int flags);
int lin_accept(LinuxSocket *skt);
int lin_send(file_desc conn, const void *response, size_t length, int flags);

int process_http(file_desc conn);
int read_method(unsigned char *buf, int buf_len, unsigned char *method);
bool validate_method(unsigned char *method, int len);
void attach_headers(Message *msg, const void *response, int opts);
int parse_version(unsigned char *buffer, size_t buf_len, int start, unsigned start *version);
bool validate_version(unsigned char *version, int len);
int parse_target(unsigned char *buffer, size_t buf_len, int start, unsigned char *target);
int get_host(unsigned char *buffer, size_t buf_len, int start, unsigned char *host);
void handle_sigint(int sig);
