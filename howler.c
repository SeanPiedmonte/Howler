#define _POSIX_C_SOURCE 200809L

#include "server.h"

LinuxSocket *lin_create_skt(uint16_t port, uint32_t ip_addr) {
    LinuxSocket *skt;
    
    file_desc fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        swictch(errno) {
            default:
                perror("ERROR [lin_create_skt]: ");
        }
    }

    skt = malloc(sizeof(LinuxSocket));
    if (skt == NULL) {
        return NULL;
    }

    skt->fd = fd;
    skt->port = port;
    skt->ip_addr = ip_addr;

    return skt;
}

long lin_send(file_desc conn, const void *response, int flags) {
    long res;
    char buffer[RESPONSE_BUFFER_LEN] = "\0";
    
    Message full_response = {0};
    full_response.buf_len = RESPONSE_BUFFER_LEN;
    full_response.buffer = buffer;

    attach_headers(&full_response, response);

    res = send(conn, full_response.buffer, full_response.buf_len, flags);
    if (res == -1) {
        perror("[lin_send]: ");
        return res;
    }

    return res;
}

long lin_recv_msg(file_desc conn, Message *msg, int flags) {
    long size = recv(conn, msg->buffer, msg->buf_len, flags);
    if (size < 0) {
        /* signals an error */
        perror("ERROR [lin_recv_msg]: ");
        return -1;
    }

    return size;
}

long lin_bind(LinuxSocket *skt) {    
    struct sockaddr_in addr;
    int opt = 1;
    long res = setsockopt(skt->fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (res < 0) {
        perror("[lin_bind]: ");
        shutdown(skt->fd, 2);
        return res;
    }

    addr.sin_family = AF_INET;
    addr.sin_port   = htons(skt->port);
    addr.sin_addr.s_addr = htonl(skt->ip_addr);

    res = bind(skt->fd, (struct sockaddr *)&addr, sizeof(addr));
    if (res == -1) {
        switch(errno) {
            default:
                perror("ERROR [lin_bind]: ");
        }
    }

    return res;
}

int lin_accept(LinuxSocket *skt) {
    int res;
    size_t addr_size;
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port   = htons(skt->port);
    addr.sin_addr.s_addr = htonl(skt->ip_addr);

    addr_size = sizeof(addr);
    res = accept(skt->fd, (struct sockaddr *)&addr, (socklen_t *)(&addr_size));
    if (res == -1) {
        switch (errno) {
            default:
                perror("ERROR [lin_accept]: ");
        }
    }

    return res;
}

long lin_listen(LinuxSocket *skt, int backlog) {
    long res = listen(skt->fd, backlog);
    if (res == -1) {
        switch(errno) {
            default:
                perror("ERROR [lin_listen]: ");
        }
    }
    return res;
}

long process_http(file_desc conn) {
    long res;
    size_t read, par_read, var_read;
    char buffer[MESSAGE_BUFFER_LEN], method[METHOD_BUFFER_SIZE], target[100];
    char version[10], response[1000];
    bool valid;
    
    Message *msg = malloc(sizeof(Message));
    if (msg == NULL) {
        printf("[process_http] ERROR: Unable to Create Msg\n");
        return -1;
    }

    msg->buf_len = MESSAGE_BUFFER_LEN;
    msg->buffer = buffer;

    res = lin_recv_msg(conn, msg, 0);
    if (res == -1) {
        return res;
    }
    
    read = read_method((char *)msg->buffer, msg->buf_len, method, METHOD_BUFFER_SIZE);
    if (read == 0) {
        perror("[process_http] ERROR: NO METHOD TO READ");
        return -1;
    }

    valid = validate_method(method, read);
    if (!valid) {
        perror("[process_http] ERROR: INVALID METHOD");
        return -1;
    }
    read++;
    
    par_read = parse_target((char *)msg->buffer, msg->buf_len, read, target);
    if (par_read == 0) {
        perror("[process_http] NO DATA READ");
        return -1;
    }
    
    read += par_read+1;
    
    ver_read = parse_version((char *)msg->buffer, msg->buf_len, read, version);
    if (ver_read == 0) {
        perror("NO VERSION READ");
        return -1;
    }
    read += ver_read;

    snprintf((char *)response, 1000, "METHOD: %s\nTARGET: %s\nVERSION: %s\n", method, target, version);

    res = lin_send(conn, response, 0);
    if (res == -1) {
        return res;
    }

    close(conn);
    free(msg);

    return 0;
}

size_t parse_version(char *buffer, size_t buf_len, size_t start, char *version) {
    size_t i;
    int res;

    if (buffer == NULL) {
        return 0;
    }

    if (version == NULL) {
        return 0;
    }
    
    if (buf_len == 0) {
        return 0;
    }

    i = start;
    while (i < buf_len) {
        if (buffer[i] == '\n' || buffer[i] == '\r') {
            break;
        }
        i++;
    }

    strncpy((char *)version, (char *)(buffer + start), i-start);
    version[i-start] = '\0';
    
    res = validate_version(version, i-start);
    if (!res) {
        printf("[parse_version]: Invalid Version: %s\n", version);
        return 0;
    }
    
    return i-start;
}

bool validate_version(const char *version, size_t len) {
    if (len < 6 || len > 8) {
        printf("[validate_version] LENGTH: %d\n", len);
        return FALSE;
    }

    if (strncmp(version, "HTTP/", 5)) {
        printf("[validate_version] VERSION PREFIX HTTP FAILED\n");
        return FALSE;
    }

    if (len == 8) {
        return !(strncmp(version+5, "1.1", 3));
    } else {
        return version[5] == "2" || version[5] == "3";
    }
}

size_t parse_target(char *buffer, size_t buf_len, size_t start, char *target) {
    size_t i;

    if (buffer == NULL) {
        printf("BUFFER is NULL\n");
        return 0;
    }

    if (target == NULL) {
        printf("VERSION is NULL\n");
        return 0;
    }
    
    if (buf_len == 0) {
        return 0;
    }

    i = start;
    while (i < buf_len) {
        if (buffer[i] == ' ') {
            break;
        } else {
            i++;
        }
    }

    strncpy(target, buffer + start, i-start);
    target[i-start] = '\0';
    
    return i-start;
}

void attach_headers(Message *msg, const void *response) {
    size_t length = strlen(response);
    snprintf((char *)msg->buffer, msg->buf_len, "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\nContent-Length: %d\r\n\r\n%s\r\n",
        length, (char *)response);
    msg->buf_len = strlen(msg->buffer);
}

size_t read_method(char *buf, size_t buf_len, char *method, size_t met_len) {
    size_t i;
    if (buf_len == 0 || met_len == 0) {
        return 0;
    }

    for (i = 0; i < buf_len && i < 8 && i < met_len; i++) {
        if (buf[i] == ' ') {
            break;
        }
        method[i] = buf[i];
    }
    method[i] = '\0';
    return i;
}

bool validate_method(const char *method, size_t len) {
    bool put, get, head, post, options, connect;
    switch(len) {
        case 3:
            put = (method[0] = 'P' && method[1] == 'U' && method[2] == 'T');
            get = (method[0] = 'G' && method[1] == 'E' && method[2] == 'T');
            return get || put;
        case 4:
            head = (method[0] = 'H' && method[1] == 'E' && method[2] == 'A'
                && method[3] == 'D');
            post = (method[0] = 'P' && method[1] == 'O' && method[2] == 'S'
                && method[3] == 'T');
            return head || post;
        case 5:
            return method[0] == 'T' && method[1] == 'R' && method[2] == 'A'
                && method[3] == 'C' && method[4] == 'E';
        case 6:    
            return method[0] == 'D' && method[1] == 'E' && method[2] == 'L'
                && method[3] == 'E' && method[4] == 'T' && method[5] == 'E';
        case 7:
            options = (method[0] == 'O' && method[1] == 'P' && method[2] == 'T'
                && method[3] == 'I' && method[4] == 'O' && method[5] == 'N'
                && method[6] == 'S';
            connect = (method[0] == 'C' && method[1] == 'O' && method[2] == 'N'
                && method[3] == 'N' && method[4] == 'E' && method[5] == 'C'
                && method[6] == 'T';
            return options || connect;
        default:
            return FALSE;
    }
}

size_t get_host(char *buffer, size_t buf_len, size_t start, char *host, size_t host_len) {
    size_t i = start+6; /* We want t oskip past the "Host:" and get to the actual host */
    size_t buf_pos = 0;

    if (buffer == NULL) {
        return 0;
    }

    if (host == NULL) {
        return 0;
    }

    while (i < buf_len && buf_pos < host_len-1) {
        if (buffer[i] == '\r' || buffer[i] == '\n') {
            break;
        }
        host[buf_pos++] = buffer[i++];
    }
    host[buf_pos+1] = '\0';
    return i-start;
}

volatile sig_atomic_t keep_running = 1;
void handle_sigint(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        keep_running = 0;
    }
}
