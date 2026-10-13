#include "server.h"
#include "conv_utils.h"

int main(int argc, char **argv) {
    STOLResult port;
    long res, bind_res, listen_res;
    int acc;
    struct sigaction sa;
    char *port_str;
    LinuxSocket *skt;

    if (argc < 2) {
        printf("USAGE: ./server <port>\n");
        return 1;
    }

    port_str = argv[1];
    port     = str_to_long(port_str, NO_OPTS);
    if (port.error) {
        printf("ISSUE when grabbing the port number from args\n");
        return 1;
    }
    
    sa.sa_handler = handle_sigint;
    sa.sa_flags   = 0;
    sigemptyset(&sa.sa_mask);

    skt = lin_create_skt((uint16_t)port.val, 0);
    if (skt == NULL) {
        return 3;
    }

    bind_res = lin_bind(skt);
    if (bind_res == -1) {
        free(skt);
        return 1;
    }

    listen_res = lin_listen(skt, 0);
    if (listen_res == -1) {
        free(skt);
        return 2;
    }

    printf("Listening on Post: %d\n", skt->port);
    
    while (keep_running) {
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
