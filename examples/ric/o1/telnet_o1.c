#include "telnet_o1.h"

typedef struct {
    int   fd;
    char *json_msg;
} client_ctx_t;

static void *handle_incoming(void *arg) {
    client_ctx_t *ctx = (client_ctx_t *)arg;
    int   fd       = ctx->fd;
    char *json_msg = ctx->json_msg;
    free(ctx);

    int keepalive = 1;
    int keepidle  = 10;
    int keepintvl = 5;
    int keepcnt   = 3;
    setsockopt(fd, SOL_SOCKET,  SO_KEEPALIVE,  &keepalive, sizeof(keepalive));
    setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE,  &keepidle,  sizeof(keepidle));
    setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &keepintvl, sizeof(keepintvl));
    setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT,   &keepcnt,   sizeof(keepcnt));

    char buf[CMD_BUF_SIZE];

    while (1) {
        ssize_t n = recv(fd, buf, sizeof(buf) - 1, 0);
        if (n <= 0) {
            if (n == 0)
                printf("[SERVER] Client disconnected cleanly\n");
            else if (n <0 )
                perror("[SERVER] recv — connection lost");
            break;
        }

        buf[n] = '\0';
        printf("[SERVER] Received command: %s\n", buf);

        ssize_t sent = send(fd, json_msg, strlen(json_msg), 0);
        if (sent < 0) {
            perror("[SERVER] send — connection lost");
            break;
        }

        printf("[SERVER] Sent back json_msg (%zd bytes)\n", sent);
    }

    close(fd);
    return NULL;
}

void *telnet_server_thread(void *arg) {
    (void)arg;

  char *json_msg =
"{ \
  \"o1-config\": { \
    \"NetworkFunctions\": [{ \
      \"nodeId\": 1, \
      \"nodeName\": \"flexRIC\", \
      \"vendor\": \"OpenAirInterface5g\", \
      \"nodeType\": \"Near-RT-RIC\" \
      } \
    ], \
    \"device\": { \
      \"flexricId\": 1, \
      \"flexricName\": \"flexRIC\", \
      \"vendor\": \"OpenAirInterface\" \
    } \
  }, \
  \"O1-Operational\": { \
  } \
}";
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) { perror("socket"); return NULL; }

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {
        .sin_family      = AF_INET,
        .sin_port        = htons(SERVER_PORT),
        .sin_addr.s_addr = INADDR_ANY
    };

    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind"); close(listen_fd); return NULL;
    }
    if (listen(listen_fd, LISTEN_BACKLOG) < 0) {
        perror("listen"); close(listen_fd); return NULL;
    }

    printf("[SERVER] Listening on port %d\n", SERVER_PORT);

    while (1) {
        struct sockaddr_in peer_addr;
        socklen_t peer_len = sizeof(peer_addr);

        int incoming = accept(listen_fd, (struct sockaddr *)&peer_addr, &peer_len);
        if (incoming < 0) { perror("accept"); continue; }

        printf("[SERVER] Accepted connection from %s\n",
               inet_ntoa(peer_addr.sin_addr));

        client_ctx_t *ctx = malloc(sizeof(client_ctx_t));
        ctx->fd       = incoming;
        ctx->json_msg = json_msg;

        pthread_t tid;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        pthread_create(&tid, &attr, handle_incoming, ctx);
        pthread_attr_destroy(&attr);
    }

    close(listen_fd);
    return NULL;
}