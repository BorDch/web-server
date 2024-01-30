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

	    printf("file: %s\n", request.path);
	    
	    
	    
	    if (strcmp(request.path, "/") == 0) {
	    	printf("\nclient request\n");
	    	printf("file: %s\n", request.path);
	    	request.path[0] = '\0'; 
	    	handle_client_request(request, client_socket);	
	    // Method 'Get' checking
	    } else if(strcmp(request.method, "GET") == 0) {
	    	printf("\nGET\n"); 
		
		if (request.path[0] == '/' && request.path[1] != '?') {
			memmove(request.path, request.path + 1, strlen(request.path));
		} else if(request.path[1] == '?') {
			char* found = NULL;
			char* sub_str = strchr(request.path, '=');
			if (sub_str != NULL) {
				memmove(request.path, sub_str + 1, strlen(sub_str));
			}
			
			if (strstr(request.path, "%2F") != NULL) {
				
				char* ptr1 = request.path;
				char* ptr2 = request.path;
				
				while (*ptr1) {
					if (strncmp(ptr1, "%2F", 3) == 0) {
						*ptr2++ = '/';
            					ptr1 += 3;
					} else {
					    *ptr2++ = *ptr1++;
					}
				} *ptr2 = '\0';
			}
		}
		
		printf("Parsed file path: %s\n", request.path);
		
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
