#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "server.h"


#define MAX_SIZE 1024


volatile sig_atomic_t flag = 1;

void handler(int) {
	flag = 0;
}

void parse_request(const char *buffer, char *method, char *path, char *version) {
	int i = 0, j = 0;

	// Method parsing
	while (buffer[i] != ' ' && buffer[i] != '\0') {
		method[j++] = buffer[i++];
	}
	
	method[j] = '\0';

	i++;
	j = 0;

	// Path parsing
	while (buffer[i] != ' ' && buffer[i] != '\0') {
		path[j++] = buffer[i++];
	}
	
	path[j] = '\0';

	i++;
	j = 0;

	// Version parsing
	while (buffer[i] != '\r' && buffer[i] != '\n' && buffer[i] != '\0') {
		version[j++] = buffer[i++];
	}
	
	version[j] = '\0';
}



int main() {
	int server_socket;

	if (server_init(&server_socket) == EXIT_FAILURE) {
		return 1;
	}

	signal(SIGINT, handler);

	while (flag) {
		struct sockaddr_in client_addr;
		socklen_t client_addr_len = sizeof(client_addr);
		int client_socket = accept(server_socket, (struct sockaddr*) &client_addr, &client_addr_len);
		
		if (client_socket == -1) {
			perror("server: accept: ");
			return 1;
		}

		struct HTTP_Request *request = (struct HTTP_Request *)malloc(sizeof(struct HTTP_Request));
		
		if (request == NULL) {
			perror("server: malloc: ");
			close(client_socket);
			continue;
		}

		request->method = (char *)malloc(10 * sizeof(char));
		
		if (request->method == NULL) {
			perror("server: malloc: ");
			close(client_socket);
			continue;
		}

		request->path = (char *)malloc(255 * sizeof(char));
		
		if (request->path == NULL) {
			perror("server: malloc: ");
			close(client_socket);
			continue;
		}

		request->version = (char *)malloc(10 * sizeof(char));
		
		if (request->version == NULL) {
			perror("server: malloc: ");
			close(client_socket);
			continue;
		}
 
		// Request reading from client
		char* buffer = malloc(MAX_SIZE);
		ssize_t buf_size;
		ssize_t bytes_read;
		int total_size = 0;    

		ioctl(client_socket, FIONREAD, &buf_size);
		printf("\nBufsize is %ld\n", buf_size);
    
		while (1) {
			if (total_size > 0) break;

			memset(buffer, 0, MAX_SIZE);

			if ((bytes_read = read(client_socket, buffer, 1024)) < 0) {
				sleep(1);
				perror("Nothing was read\n");
				break;
				
			} else if (bytes_read == 0) {
				printf("It's already loaded on the website\n");
				break;
				
			} else {
				total_size += bytes_read;
				printf("%s ", buffer);

				if (total_size >= MAX_SIZE) {
					size_t new_size = total_size * 2;
					buffer = realloc(buffer, new_size);
					
					if (buffer == NULL) {
						perror("Reallocation memory");
						break;
					}
				}
			}
		}
    
		if (bytes_read < 0) {
			perror("server: recv: ");
			free(request->method);
			free(request->path);
			free(request->version);
			free(request);
			close(client_socket);
			free(buffer);
			continue;
		}

		// Analyze request
		printf("Buffer is %s\n", buffer);

		parse_request(buffer, request->method, request->path, request->version);

		printf("file: %s\n", request->path);

		// GET / file_name/version
		if (strcmp(request->path, "/") == 0) {
			printf("\nclient request\n");
			request->path[0] = '\0';
			handle_client_request(*request, client_socket);

		} else if(strcmp(request->method, "GET") == 0) {
			printf("\nGET\n");

			if (request->path[0] == '/' && request->path[1] != '?') {
				memmove(request->path, request->path + 1, strlen(request->path));
			
			} else if(request->path[1] == '?') {
				char* sub_str = strchr(request->path, '=');
				
				if (sub_str != NULL) {
					memmove(request->path, sub_str + 1, strlen(sub_str));
				}

				if (strstr(request->path, "%2F") != NULL) {
					char* ptr1 = request->path;
					char* ptr2 = request->path;

					while (*ptr1) { 
						if (strncmp(ptr1, "%2F", 3) == 0) {
							*ptr2++ = '/';
							ptr1 += 3;
							
						} else {
							*ptr2++ = *ptr1++;
						}
					}
					
					*ptr2 = '\0';
				}
			}

			printf("Parsed file path: %s\n", request->path);

			handle_get_request(*request, client_socket);
		} else {
			// Not implemented method, send error: 405 Method Not Allowed
			handle_not_allowed_method(client_socket);
		}

		free(request->method);
		free(request->path);
		free(request->version);
		free(request);
		free(buffer);

		shutdown(client_socket, SHUT_RDWR);
		close(client_socket);
	}

	fprintf(stdout, "\nServer is shutting down\n");
	close(server_socket);

	return 0;
}
