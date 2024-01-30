#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include <sys/select.h>
#include <sys/stat.h>

#include "server.c"



void server_init(int *server_socket);
void handle_get_request(struct HTTP_Request request, int client_socket);
void handle_not_allowed_method(int client_socket);
void handle_client_request(struct HTTP_Request request, int client_socket);
