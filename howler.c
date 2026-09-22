#include "server.h"

int lin_recv_msg(LinuxSocket *skt, Message *msg, int flags) {
    int size = recv(skt->fd, msg->buffer, msg->buf_len, flags);
    if (size < 0) {
        /* signals an error */
        perror("ERROR [lin_recv_msg]: ");
        return 0;
    }

    return size;
}

LinuxSocket *lin_create_skt(uint16_t port, uint32_t ip_addr) {
    file_desc fd = socket(AF_INET, SOCK_STREAM, 0);
    if (res == -1) {
        swictch(errno) {
            default:
                perror("ERROR [lin_create_skt]: ");
        }
    }

    LinuxSocket *skt = malloc(sizeof(LinuxSocket));
    if (skt == NULL) {
        return NULL;
    }

    skt->fd = fd;
    skt->port = port;
    skt->ip_addr = ip_addr;

    return skt;
}

int lin_bind(file_desc skt, uint16_t port, uint32_t ip_addr) {
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    addr.sin_addr.s_addr = htonl(ip_addr);

    int res = bind(skt, (struct sockaddr *)&addr, sizeof(addr));
    if (res == -1) {
        switch(errno) {
            default:
                perror("ERROR [lin_bind]: ");
        }
    }

    return res;
}

int lin_accept(LinuxSocket *skt) {
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port   = htons(skt->port);
    addr.sin_addr.s_addr = htonl(skt->ip_addr);

    int addr_size = sizeof(addr);
    int res = accept(skt, (struct sockaddr *)&addr, &addr_size);
    if (res == -1) {
        switch (errno) {
            default:
                perror("ERROR [lin_accept]: ");
        }
    }

    return res;
}

int lin_listen(file_desc skt, int backlog) {
    int res = listen(skt->fd, backlog);
    if (res == -1) {
        switch(errno) {
            default:
                perror("ERROR [lin_listen]: ");
        }
    }
    return res;
}

int process_http(file_desc conn) {
    Message *msg = malloc(sizeof(Message));
    if (msg == NULL) {
        printf("[process_http] ERROR: Unable to Create Msg\n");
        return -1;
    }

    msg->buf_len = MESSAGE_BUFFER_LEN;
    unsigned char buffer[MESSAGE_BUFFER_LEN];
    msg->buffer = buffer;

    int res = lin_recv_msg(conn, msg, 0);
    if (res == -1) {
        return res;
    }

    unsigned char method[8] = '\0';
    int met_read = read_method((unsigned char *)msg->buffer, msg->buf_len, method);
    if (met_read == 0) {
        perror("[process_http] ERROR: NO METHOD TO READ");
        return -1;
    }

    bool valid = validate_method(method, met_read);
    if (!valid) {
        perror("[process_http] ERROR: INVALID METHOD");
        return -1;
    }

    printf("METHOD: %s\n", method);

    unsigned char target[100] = "\0";
    int par_read = parse_target((unsigned char *)msg->buffer, msg->buf_len, met_read, target);
    if (par_read == 0) {
        perror("[process_http] NO DATA READ");
        return -1;
    }

    printf("TARGET: %s\n", target);

    unsigned char version[100]; /* need to create version parsing */

    unsigned char response[1000] = "\0";
    snprintf(response, 1000, "METHOD: %s\nTARGET: %s\n", method, target);

    res = lin_send(conn, response, strlen(response), 0);
    if (res == -1) {
        return res;
    }

    close(conn);
    free(msg);

    return 0;
}

int lin_send(file_desc conn, const void *response, size_t length, int flags) {
    unsigned char buffer[RESPONSE_BUFFER_LEN] = "\0";
    Message full_response = {0};
    full_response.buf_len = RESPONSE_BUFFER_LEN;
    full_response.buffer = buffer;

    attach_headers(&full_response, response, 0);

    printf("FULL RESPONSE: %s\n", (unsigned char *)full_response.buffer);
    int res = send(conn, full_response.buffer, full_response.buf_len, flags);
    if (res == -1) {
        perror("[lin_send]: ");
        return res;
    }

    return res;
}

void attach_headers(Message *msg, const void *response, int opts) {
    size_t length = strlen(response);
    snprintf((unsigned char *)msg->buffer, msg->buf_len, "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\nContent-Length: %d\r\n\r\n%s\r\n",
        length, response);
    msg->buf_len = strlen(msg->buffer);
}

int parse_target(unsigned char *buffer, int buf_len, int start, unsigned char *target) {
    if (buf_len == 0) {
        return 0;
    }

    int i = start+1;
    for(;;) {
        if (buffer[i] == ' ') {
            i--;
            break;
        }
        i++;
    }

    strncpy(target, buffer + start + 1, i-1);

    return i-start;
}

bool validate_method(unsigned char *method, int len) {
    switch(len) {
        case 3:
            bool put = (method[0] = 'P' && method[1] == 'U' && method[2] == 'T');
            bool get = (method[0] = 'G' && method[1] == 'E' && method[2] == 'T');
            return get || put;
        case 4:
            bool head = (method[0] = 'H' && method[1] == 'E' && method[2] == 'A'
                && method[3] == 'T');
            bool post = (method[0] = 'P' && method[1] == 'O' && method[2] == 'S'
                && method[3] == 'T');
            return head || post;
        case 5:
            return method[0] == 'T' && method[1] == 'R' && method[2] == 'A'
                && method[3] == 'C' && method[4] == 'E';
        case 6:    
            return method[0] == 'D' && method[1] == 'E' && method[2] == 'L'
                && method[3] == 'E' && method[4] == 'T' && method[5] == 'E';
        case 7:
            bool options = (method[0] == 'O' && method[1] == 'P' && method[2] == 'T'
                && method[3] == 'I' && method[4] == 'O' && method[5] == 'N'
                && method[6] == 'S';
            bool connect = (method[0] == 'C' && method[1] == 'O' && method[2] == 'N'
                && method[3] == 'N' && method[4] == 'E' && method[5] == 'C'
                && method[6] == 'T';
            return options || connect;
        default:
            return FALSE;
    }
}

int read_method(unsigned char *buf, int buf_len, unsigned char *method) {
    int i;
    if (buf_len == 0) {
        return 0;
    }

    for (i = 0; i < buf_len; i++) {
        if (buf[i] == ' ') {
            return i;
        }

        method[i] = buf[i];
    }
}
