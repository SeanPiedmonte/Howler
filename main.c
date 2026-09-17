#include "server.h"

int main(int argc, char **argv) {
    LinuxSocket *skt = lin_create_skt(8080, 0);
    if (skt == NULL) {
        perror("ERROR [main]: ");
    }

    int bind_res = lin_bind(skt);
    if (bind_res == -1) {
        return 1;
    }

    int listen_res = lin_listen(skt, 0);
    if (listen_res == -1) {
        return 2;
    }

    int acc;
    while (TRUE) {
        acc = lin_accept(skt);
        if (acc == -1) {
            printf("Issue with socket\n");
        }

        /*Implement a function to handle HTTP requests*/
    }
}
