#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <signal.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "server.h"


int main() {
	int server_socket;

	if (server_init(&server_socket) == EXIT_FAILURE) {
		return 1;
	}

	signal(SIGINT, handler);

	if (server_run(server_socket) == 1) {
		printf("Server has been finished by error\n");
	}

	fprintf(stdout, "\nServer is shutting down\n");
	close(server_socket);

	return 0;
}
