#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include <sys/select.h>
#include <sys/stat.h>

#include "server.c"


// For server launching
int server_init(int *server_socket);
int server_run(int server_socket);

// For request handling
void parse_request(const char *buffer, char *method, char *path, char *version);
void handle_not_allowed_method(int client_socket);
void handle_not_exist_error(int client_socket, char date_header[], char allow_header[]);
void handle_internal_server_error(int client_socket, char date_header[], char allow_header[]);
void handle_get_request(struct HTTP_Request request, int client_socket);
void handle_client_request(struct HTTP_Request request, int client_socket);


// For threads
void *client_handler(void *arg);
