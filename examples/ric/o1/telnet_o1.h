#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <netinet/tcp.h>

#define SERVER_PORT   9090
#define REMOTE_IP     "127.0.0.1"
#define LISTEN_BACKLOG 5
#define CMD_BUF_SIZE  4096

void *telnet_server_thread(void *arg);