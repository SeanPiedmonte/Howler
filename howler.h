#ifndef SERVER_H
#define SERVER_H

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
#define METHOD_BUFFER_SIZE 8

typedef int file_desc;
typedef int bool;

typedef enum VERSION {
    HTTP1_1 = 1,
    HTTP2   = 2,
    HTTP3   = 3
} VERSION;

typedef struct LinuxSocket {
    file_desc fd;
    
    uint16_t  port;
    uint32_t  ip_addr;
} LinuxSocket;

typedef struct Message {
    size_t buf_len;
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
    STATUSOK = 200
};

void get();
void put();
void del();
void post();

/* Sockets Specific Functions */
LinuxSocket *lin_create_skt(uint16_t port, uint32_t ip_addr);
long lin_bind(LinuxSocket *skt);
long lin_listen(LinuxSocket *skt, int backlog);
long lin_recv_msg(file_desc conn, Message *msg, int flags);
int lin_accept(LinuxSocket *skt);
long lin_send(file_desc conn, const void *response, int flags);

long process_http(file_desc conn);
size_t read_method(char *buf, size_t buf_len, char *method, size_t met_size);
bool validate_method(const char *method, size_t len);
void attach_headers(Message *msg, const void *response);
size_t parse_version(char *buffer, size_t buf_len, size_t start, start *version);
bool validate_version(const char *version, size_t len);
size_t parse_target(char *buffer, size_t buf_len, size_t start, char *target);
size_t get_host(char *buffer, size_t buf_len, size_t start, char *host, size_t host_len);

/* Miscellaneous Utility Functions */
void handle_sigint(int sig);

#endif
