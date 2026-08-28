#include "server.h"

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