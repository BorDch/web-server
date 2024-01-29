#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>

#include <sys/select.h>
#include <sys/stat.h>

#include <sys/socket.h>
#include <netinet/in.h>


#define MAX_CLIENTS 5
#define PORT 8080


struct HTTP_Request {
	char method[10]; // HTTP-methods: GET, HEAD, POST, ...
	char path[255];  // Path to requested resource
	char version[10]; // Protocol version: 1.1 or 1.0 for example
};



// Function for initializing the server using sockets
void server_init(int *server_socket) {
    if ((*server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("server: socket: ");
        exit(EXIT_FAILURE);
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
        exit(EXIT_FAILURE);
    }

    if (listen(*server_socket, MAX_CLIENTS) == -1) {
        perror("server: listen: ");
        exit(EXIT_FAILURE);
    }
}


void handle_get_request(struct HTTP_Request request, int client_socket) {
    // Open requested file
    int fd = open(request.path, O_RDONLY);
    if (fd != -1) {
        struct stat filestats;
        // Getting information about file
        if (stat(request.path, &filestats) != -1) {
            char *body = (char*) calloc(filestats.st_size, sizeof(char));
	    read(fd, body, filestats.st_size);
            //char response[] = "HTTP/1.1 200 OK\r\nServer: Custom HTTP server\r\nContent-Type: text/html\r\n\r\n";
            char response[] = "HTTP/1.1 200 OK\nServer: Custom HTTP server\nContent-Type: text/html\nContent-length: 72\n\n";
            size_t size = strlen(response) + strlen(body);
	    char *final = (char*) calloc(size, sizeof(char));
	    strcat(final, response);
	    strcat(final, body);
            // Send responce title to client
            write(client_socket, final, size);
            
            printf("Response has been successfully sent to client:\n%s", response);
        } else {
            // Getting information error
            char response[] = "HTTP/1.1 500 Internal Server Error\r\n"
            	  "Server: Custom HTTP server\r\n"
                  "Content-Type: text/html\r\n\r\n"
                  "<html><head><title>500 Internal Server Error</title></head>"
                  "<body><h1>500 Internal Server Error</h1><p>Internal server error."
                  "Please try to repeat the request later or contact the site administrator."
                  "</p></body></html>";
            
            write(client_socket, response, strlen(response));
            printf("Error 500 Internal Server Error occurred, the following response code was sent:\n%s", response);
        }
        close(fd);
    } else {
        // If requested file doesn't exist
        char response[] = "HTTP/1.1 404 Not Found\r\n"
              "Server: Custom HTTP server\r\n"
              "Content-Type: text/html\r\n\r\n"
              "<html><head><title>404 Not Found</title></head>"
              "<body><h1>404 Not Found</h1><p>The requested URL was not found on this server."
              "</p></body></html>";
        
        write(client_socket, response, strlen(response));
        printf("File was not found, the '404 Not Found' response code was sent:\n%s", response);
    }
}


void handle_not_allowed_method(int client_socket) {
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
