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

    int res = accept(skt, (struct sockaddr *)&addr, sizeof(addr));
    if (res == -1) {
        switch (errno) {
            default:
                perror("ERROR [lin_accept]: ");
        }
    }

    return res;
}

int lin_listen(file_desc skt, int backlog) {
    int res = listen(skt, backlog);
    if (res == -1) {
        switch(errno) {
            default:
                perror("ERROR [lin_listen]: ");
        }
    }

    // Need to work on this file
}
