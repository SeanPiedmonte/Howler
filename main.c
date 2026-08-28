#include "server.h"

int main(int argc, char **argv) {
    file_desc skt = socket(AF_INET, SOCK_STREAM, 0);
    if (skt == -1) {
        perror("ERROR [main]: ");
    }

    int bind_res = lin_bind(skt, 80, 0);
    if (bind_res == -1) {
        return 1;
    }

    int listen_res = lin_listen(skt, 0);
    if (listen_res == -1) {
        return 2;
    }
}