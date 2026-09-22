#include "server.h"

int main(int argc, char **argv) {
    LinuxSocket *skt = lin_create_skt(8080, 0);
    if (skt == NULL) {
        return 3;
    }

    int bind_res = lin_bind(skt);
    if (bind_res == -1) {
        free(skt);
        return 1;
    }

    int listen_res = lin_listen(skt, 0);
    if (listen_res == -1) {
        free(skt);
        return 2;
    }

    printf("Listening on Post: %d\n", skt->port);
    
    int acc, res;
    while (TRUE) {
        acc = lin_accept(skt);
        if (acc == -1) {
            printf("Issue with socket\n");
        }

        res = process_http(acc);
        if (res == -1) {
            break;
        }
    }

    shutdown(skt->fd, 2);
    free(skt);
}
