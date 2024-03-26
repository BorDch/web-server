#pragma once

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


#define MAX_CLIENTS 5
#define MAX_SIZE 1024
#define PORT 8080


struct HTTP_Request {
        char* method; // HTTP-methods: GET, HEAD, POST, ...
        char* path;  // Path to requested resource
        char* version; // Protocol version: 1.1 or 1.0 for example
        char* name;   // Name of the user
        char* dob; // Date of birth of the user
};


struct ClientSocketArgs {
	int client_socket;
	fd_set* readfds;
};


void handler(int);

// For server launching
int server_init(int *server_socket);
int server_run(int server_socket);

// For request handling
void parse_request(const char *buffer, struct HTTP_Request* http_request);
void send_html_form(int client_socket);

void handle_not_allowed_method(int client_socket);
void handle_not_exist_error(int client_socket, char date_header[], char allow_header[]);
void handle_internal_server_error(int client_socket, char date_header[], char allow_header[]);
void handle_get_request(struct HTTP_Request request, int client_socket);
void handle_client_request(struct HTTP_Request request, int client_socket);


// For threads
void *client_handler(void *arg);
