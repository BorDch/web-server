#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "server.h"


volatile sig_atomic_t flag = 1;

void handler(int) {
	flag = 0;
}

int main() {
	int server_socket;
	server_init(&server_socket);

	signal(SIGINT, handler);

	while (flag) {
	    struct sockaddr_in client_addr;
	    socklen_t client_addr_len = sizeof(client_addr);
	    int client_socket = accept(server_socket, (struct sockaddr*) &client_addr, &client_addr_len);
	    if (client_socket == -1) {
		perror("server: accept: ");
		return 1;
	    }

	    // Request reading from client
	    char buffer[1024] = {0};
	    ssize_t bytes_read = recv(client_socket, buffer, sizeof(buffer), 0);
	    if (bytes_read <= 0) {
		perror("server: recv: ");
		close(client_socket);
		continue;
	    }
	    printf("%s\n", buffer);

	    // Analyze request
	    struct HTTP_Request request;
	    sscanf(buffer, "%s %s %s", request.method, request.path, request.version);

	    //printf("file: %s\n", request.path);
	    
	    // Method 'Get' checking
	    if (strcmp(request.method, "GET") == 0) {
	    	printf("\nGET\n"); 
		
		if (request.path[0] == '/') {
			memmove(request.path, request.path + 1, strlen(request.path));
			//printf("file path is changed: %s\n", request.path);
		}
		
		handle_get_request(request, client_socket);
	    } else {
		// Not implemented method, send error: 405 Method Not Allowed
		handle_not_allowed_method(client_socket);
	    }

	    shutdown(client_socket, SHUT_RDWR);
	    close(client_socket);
	}

	fprintf(stdout, "\nServer is shutting down\n");
	close(server_socket);
	return 0;
}
