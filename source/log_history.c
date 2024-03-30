#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "server.h"


void log_request(const char* user, const char* request) {
	FILE *log_file = fopen("history.txt", "a");
	if (!log_file) {
		perror("log_request: fopen");
		return;
	}
	
	time_t current_time;
	time(&current_time);
	char* time_str = ctime(&current_time);
	time_str[strlen(time_str) - 1] = '\0';
	
	fprintf(log_file, "[%s] Request info: UserName: %s --- Request: %s\n", time_str, user, request);
	
	fclose(log_file);
}
