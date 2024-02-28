#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <pthread.h>

#include <sys/select.h>
#include <sys/stat.h>

#include <sys/socket.h>
#include <netinet/in.h>


#define MAX_CLIENTS 5
#define MAX_SIZE 1024
#define PORT 8080


volatile sig_atomic_t flag = 1;

void handler(int) {
	flag = 0;
}

struct HTTP_Request {
        char* method; // HTTP-methods: GET, HEAD, POST, ...
        char* path;  // Path to requested resource
        char* version; // Protocol version: 1.1 or 1.0 for example
};


struct ClientSocketArgs {
	int client_socket;
	int* client_sockets;
	fd_set* readfds;
};


// Function for initializing the server using sockets
int server_init(int *server_socket) {
	if ((*server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
                perror("server: socket: ");
                return EXIT_FAILURE;
        }

        struct sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));

        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(PORT);

        int opt = 1;
        setsockopt(*server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        if (bind(*server_socket, (struct sockaddr*) &server_addr, sizeof(server_addr)) == -1) {
                perror("server: bind: ");
                return EXIT_FAILURE;
        }

        if (listen(*server_socket, MAX_CLIENTS) == -1) {
                perror("server: listen: ");
                return EXIT_FAILURE;
        }
}


void handle_not_allowed_method(int client_socket) {
	// HTLM-form for displaying available methods
	
	char response[] = "HTTP/1.1 405 Method Not Allowed\r\n"
	"Server: Custom HTTP server\r\n"
	"Content-Type: text/html\r\n\r\n"
	"<html><head><title>405 Method Not Allowed</title></head>"
	"<body><h1>405 Method Not Allowed</h1><p>The requested method is not allowed for the requested URL."
	"</p><h2>Allowed Methods:</h2><ul><li>GET - Retrieve the resource</li></ul><h2>"
	"Example:</h2><p>To retrieve a resource, use the GET method with the URL of the resource."
	"</p></body></html>";

	send(client_socket, response, strlen(response), 0);
}


void handle_not_exist_error(int client_socket, char date_header[], char allow_header[]) {
	// HTLM-form for displaying an error related to the file existence
	
	char response_form[] = "HTTP/1.1 404 Not Found\r\n"
       		"Server: Custom HTTP server\r\n"
       		"Content-Type: text/html\r\n\r\n"
       		"<html><head><title>404 Not Found</title></head>"
       		"<body><h1>404 Not Found</h1><p>The requested FILE PATH was not found on this server."
       		"</p></body></html>";
       		
        send(client_socket, response_form, strlen(response_form), 0);
        
        char response[] = "HTTP/1.1 404 Not Found\n"
       		"Server: Custom HTTP server\n"
       		"Content-Type: text/html\n";
       
        printf("File was not found, the '404 Not Found' response code was sent:\n%s%s%s", response, date_header, allow_header);
}


void handle_internal_server_error(int client_socket, char date_header[], char allow_header[]) {
	// HTLM-form for displaying an error related to server internal error
	
	char response_form[] = "HTTP/1.1 500 Internal Server Error\r\n"
		"Server: Custom HTTP server\r\n"
		"Content-Type: text/html\r\n\r\n"
		"<html><head><title>500 Internal Server Error</title></head>"
		"<body><h1>500 Internal Server Error</h1><p>Internal server error." 
		"Please try to repeat the request later or contact the site administrator."
		"</p></body></html>";
            
	send(client_socket, response_form, strlen(response_form), 0);

	char response[] = "HTTP/1.1 500 Internal Server Error\n"
		"Server: Custom HTTP server\n"
		"Content-Type: text/html\n";

	printf("Error 500 Internal Server Error occurred, the following response code was sent:\n%s%s%s", response, date_header, allow_header);
}


void handle_get_request(struct HTTP_Request request, int client_socket) {
	// Open requested file
	printf("HERE FILE %s\n", request.path);
	int fd = open(request.path, O_RDONLY);

	// Make header: Date
	time_t rawtime;
	struct tm *timeinfo;
	char date_header[100];
	time(&rawtime);
	timeinfo = gmtime(&rawtime);
	strftime(date_header, sizeof(date_header), "Date: %a, %d %b %Y %H:%M:%S GMT\n", timeinfo);
    
	// Make header: Allow
	char allow_header[] = "Allow: GET\r\n";

	if (fd != -1) {
		struct stat filestats;
        	// Getting information about file
        	if (stat(request.path, &filestats) != -1) {
            		printf("successfully opened\n");
			// Read file's content
			char *body = (char*) calloc(filestats.st_size, sizeof(char));
			read(fd, body, filestats.st_size);

			// Defining the MIME-type according to the file extension
			char content_type[50];
			if (strstr(request.path, ".html")) {
				strcpy(content_type, "text/html");
                
			} else if (strstr(request.path, ".css")) {
				strcpy(content_type, "text/css");

			} else if (strstr(request.path, ".txt")) {
				strcpy(content_type, "text/plain");

			} else if (strstr(request.path, ".png")) {
				strcpy(content_type, "image/png");

			} else if (strstr(request.path, ".gif")) {
				strcpy(content_type, "image/gif");

			} else if (strstr(request.path, ".jpeg")) {
				strcpy(content_type, "image/jpeg"); 

			} else if (strstr(request.path, ".ico")) {
				strcpy(content_type, "image/vnd.microsoft.icon"); 

			} else if (strstr(request.path, ".pdf")) {
				strcpy(content_type, "application/pdf");

			} else {
				// If MIME-type is undefined, then use common type: application/octet-stream
				strcpy(content_type, "application/octet-stream");
			}

			// Make header: Content-Type
			char content_type_header[100];
			sprintf(content_type_header, "Content-Type: %s\r\n", content_type);

			// Make header: Last-modified
			char last_modified_header[100];
			strftime(last_modified_header, sizeof(last_modified_header), "Last-Modified: %a, %d %b %Y %H:%M:%S GMT\r\n", gmtime(&filestats.st_mtime));

			// Make HTTP-response
			char response[1024];
			sprintf(response, "HTTP/1.1 200 OK\nServer: Custom HTTP server\r\n%s%s%s%sContent-length: %ld\r\n\r\n", content_type_header, date_header, allow_header, last_modified_header, filestats.st_size);

			// Send responce title to client
			send(client_socket, response, strlen(response), 0);
			send(client_socket, body, filestats.st_size, 0);

			printf("Response has been successfully sent to client:\n%s", response);
			free(body);
		} else {
			// Getting information error
		   	handle_internal_server_error(client_socket, date_header, allow_header);
		}
		
		close(fd);
	} else {
		// If requested file doesn't exist     
		handle_not_exist_error(client_socket, date_header, allow_header);
	}
}


void handle_client_request(struct HTTP_Request request, int client_socket) {
	if (strcmp(request.method, "GET") == 0 && strlen(request.path) != 0) {
		if (request.path[0] == '/') {
			memmove(request.path, request.path + 1, strlen(request.path));
			printf("\nfile path is changed: %s\n", request.path);
		}
		
		handle_get_request(request, client_socket);
	
	} else if (strcmp(request.method, "GET") != 0) {
		handle_not_allowed_method(client_socket);
	}
}


// Request parse function. It reformates the client request
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


void *client_handler(void *arg) {
	struct ClientSocketArgs *client_args = (struct ClientSocketArgs *)arg;
	int client_socket = client_args->client_socket;
	int* client_sockets = client_args->client_sockets;
	fd_set* readfds = client_args->readfds;

	struct HTTP_Request *request = (struct HTTP_Request *)malloc(sizeof(struct HTTP_Request));

	if (request == NULL) {
		perror("client_handler: malloc: ");
		close(client_socket);
		pthread_exit(NULL);
	}

	request->method = (char*)malloc(10 * sizeof(char));
	request->path = (char*)malloc(255 * sizeof(char));
	request->version = (char*)malloc(10 * sizeof(char));

	char buffer[MAX_SIZE];
	ssize_t bytes_read = read(client_socket, buffer, MAX_SIZE);
	ssize_t buf_size;
				
	ioctl(sd, FIONREAD, &buf_size);
	printf("Buffer size: %ld\n", buf_size);

    
	if (bytes_read < 0) {
		perror("client_handler: read: ");
		free(request->method);
		free(request->path);
		free(request->version);
		free(request);
		close(client_socket);
		pthread_exit(NULL);
	}

	buffer[bytes_read] = '\0';
	parse_request(buffer, request->method, request->path, request->version);
	handle_client_request(*request, client_socket);

	free(request->method);
	free(request->path);
	free(request->version);
	free(request);
	close(client_socket);

	for (int i = 0; i < MAX_CLIENTS; i++) {
		if(client_sockets[i] == client_socket) {
			FD_CLR(client_socket, readfds);
			client_sockets[i] = 0;
			break;
		}
	}

	pthread_exit(NULL);
}


// Function for server process running (from request getting to send responce to client) 
int server_run(int server_socket) {
	fd_set readfds;
	int client_sockets[MAX_CLIENTS] = {0};
	int max_sd, activity, new_socket;

	while (flag) {
		FD_ZERO(&readfds);
		FD_SET(server_socket, &readfds);
		max_sd = server_socket;

		for (int i = 0; i < MAX_CLIENTS; i++) {
			if (client_sockets[i] > 0) {
				FD_SET(client_sockets[i], &readfds);

				if (client_sockets[i] > max_sd) {
					max_sd = client_sockets[i];
				}
		    	}
		}	

		errno = 0;
		activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

		if ((activity < 0) && (errno != EINTR)) {
			printf("\nselect error\nerrno - %d\n", errno);
		}

		if (FD_ISSET(server_socket, &readfds)) {
			struct sockaddr_in client_addr;
			socklen_t client_addr_len = sizeof(client_addr);
			new_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);

			if (new_socket < 0) {
				perror("server: accept: ");
				return 1;
			}

			for (int i = 0; i < MAX_CLIENTS; i++) {
				if (client_sockets[i] == 0) {
					client_sockets[i] = new_socket;
					break;
				}
			}

			struct ClientSocketArgs args;
			args.client_socket = new_socket;
			args.client_sockets = client_sockets;
			args.readfds = &readfds;

			// Create thread for client request handling
			pthread_t thread_id;
			if (pthread_create(&thread_id, NULL, client_handler, (void *)&args) != 0) {
				perror("server: pthread_create: ");
				close(new_socket);
				continue;
			}
			
			pthread_detach(thread_id); // Free thread resources after process finished
		}
	}

	return 0;
}
