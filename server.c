#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <getopt.h>
#include "common.h"
#include "camera.h"
#include "compass.h"


void signal_handler(int signum);
int split(char* str, char* str_array[]);
void parse_command(char buffer[], struct Command* command);

bool VERBOSE = false;

int loop = 1;

const char capture_command[] = "capture";
const char stop_command[] = "stop";


void signal_handler(int signum) {
	switch ( signum ) {
		case SIGKILL:
			syslog(LOG_INFO, "SIGKILL Received...\nexiting...\n");
			if (VERBOSE) printf("SIGKILL Received...\nexiting...\n");
			exit(EXIT_SUCCESS);
			break;
		case SIGTERM:
			syslog(LOG_INFO, "SIGTERM Received...\nexiting...\n");
			if (VERBOSE) printf("SIGTERM Received...\nexiting...\n");
			exit(EXIT_SUCCESS);
			break;
		case SIGUSR1:
			syslog(LOG_INFO, "SIGUSR1 Received...\nexiting...\n");
			if (VERBOSE) printf("SIGUSR1 Received...\nexiting...\n");
			exit(EXIT_SUCCESS);
			break;
		case SIGUSR2:
			syslog(LOG_INFO, "SIGUSR2 Received...\nexiting...\n");
			if (VERBOSE) printf("SIGUSR2 Received...\nexiting...\n");
			exit(EXIT_SUCCESS);
			break;
		case SIGSEGV:
			syslog(LOG_INFO, "SIGSEGV Received...\nexiting...\n");
			if (VERBOSE) printf("SIGSEGV Received...\nexiting...\n");
			exit(EXIT_FAILURE);
			break;
	}
}


int split(char* str, char* str_array[5]) {
	char* pch;
	pch = strtok(str, " ,.-");
	int i = 0;
	while (pch != NULL) {
		str_array[i] = pch;
		pch = strtok (NULL, " ,.-");
		i++;
	}
	return i;
}


void parse_command(char buffer[], struct Command* command) {

	char* command_array[5];
	int command_array_size;

	command->exp_time_us = 1000;
	command->xbin = 1;
	command->ybin = 1;

	if (strstr(buffer, capture_command) != NULL){ // capture command received
		command_array_size = split(buffer, command_array); // split the command
		command->flag = CAPTURE;
		switch (command_array_size) {
			case 2:
				command->exp_time_us = (unsigned long)atol(command_array[1]);
				break;
			case 3:
				command->exp_time_us = (unsigned long)atol(command_array[1]);
				command->xbin = (unsigned char)atoi(command_array[2]);
				break;
			case 4:
				command->exp_time_us = (unsigned long)atol(command_array[1]);
				command->xbin = (unsigned char)atoi(command_array[2]);
				command->ybin = (unsigned char)atoi(command_array[3]);
				break;
		}
	} else if (strstr(buffer, stop_command) != NULL){ // stop command received
		command->flag = STOP;
	} else {
		command->flag = INVALID;
	}

}


int main(int argc , char *argv[]) {

	signal(SIGUSR1, signal_handler);
	signal(SIGUSR2, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGKILL, signal_handler);
	signal(SIGSEGV, signal_handler);

	int option = 0;

	int port = 8000;

	while ((option = getopt(argc, argv, "hvp:")) != -1) {
		switch (option) {
		case 'v' :
			VERBOSE = true;
			break;
		case 'p' :
			port = atoi(optarg);
			break;
		case 'h' :
			printf("Usage:\tcamdaemon -h -v -p 3000\nv\t- VERBOSE\np #\t- port (default 8000)\nh\t- this help screen\n");
			syslog(LOG_INFO, "exiting...\n");
			if (VERBOSE) printf("exiting...\n");
			exit(EXIT_SUCCESS);
			break;
		default :
			printf("Usage:\tcamdaemon -h -v -p 3000\nv\t- VERBOSE\np #\t- port (default 8000)\nh\t- this help screen\n");
			syslog(LOG_ERR, "Unknown Option : %s\nexiting...\n", argv[1]);
			if (VERBOSE) printf("Unknown Option : %s\nexiting...\n", argv[1]);
			exit(EXIT_FAILURE);
			break;
		}
	}

	/* declare  socket */
	int socketfd, newsocketfd;
	socklen_t clientlen;
	struct sockaddr_in server, client;
	int iset_option = 1;
	int n, m;
	char rx_buffer[128];
	struct Command command;
	char* response;

	/* define socket */
	socketfd = socket(AF_INET, SOCK_STREAM, 0);
	// fcntl(socketfd, F_SETFL, O_NONBLOCK);
	if (socketfd < 0) {
		syslog(LOG_ERR, "error opening socket\nexiting...\n");
		if (VERBOSE) printf("error opening socket\nexiting...\n");
		exit(EXIT_FAILURE);
	} else {
		syslog(LOG_INFO, "camdaemon started on port %d\n", port);
		if (VERBOSE) printf("camdaemon started on port %d\n", port);
	}

	memset((char*)&server, 0, sizeof(server));

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(port);
	setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, (char*)&iset_option, sizeof(iset_option));

  if (bind(socketfd, (struct sockaddr*)&server, sizeof(server)) < 0) {
		syslog(LOG_ERR, "error on binding to port %d\nexiting...\n", port);
		if (VERBOSE) printf("error on binding to port %d\nexiting...\n", port);
		exit(EXIT_FAILURE);
	}

	init_camera();
	init_compass();

	listen(socketfd, 5);

  do {

		clientlen = sizeof(client);
		newsocketfd = accept(socketfd, (struct sockaddr*)&client,  &clientlen);
		in_addr_t address = client.sin_addr.s_addr;

		if (newsocketfd < 0) {
			uninit_camera();
			uninit_compass();


			syslog(LOG_ERR, "error on accepting connection\nexiting...\n");
			if (VERBOSE) printf("error on accepting connection\nexiting...\n");
			exit(EXIT_FAILURE);
		} else {
			syslog(LOG_INFO, "%d.%d.%d.%d connected\n", (address&0xFF), (address&0xFF00)>>8, (address&0xFF0000)>>16, (address&0xFF000000)>>24 );
			if (VERBOSE) printf("%d.%d.%d.%d connected\n", (address&0xFF), (address&0xFF00)>>8, (address&0xFF0000)>>16, (address&0xFF000000)>>24 );
		}


		n = read(newsocketfd, rx_buffer, 128);
		if (n < 0) {
			uninit_camera();
			uninit_compass();


			syslog(LOG_ERR, "error reading from socketfd\nexiting...\n");
			if (VERBOSE) printf("error reading from socketfd\nexiting...\n");
			exit(EXIT_FAILURE);
		} else {
			parse_command(rx_buffer, &command);

			switch (command.flag) {
				case INVALID:
					response = "invalid command\n";
					m = write(newsocketfd, response, strlen(response)); // respond with stopping message
					if (m < 0) {
						syslog(LOG_ERR, "error writing to socket\nexiting...\n");
						if (VERBOSE) printf("error writing to socket\nexiting...\n");
						exit(EXIT_FAILURE);
					} else {
						syslog(LOG_INFO, "invalid command\n");
						if (VERBOSE) printf("invalid command\n");
					}
					break;

				case STOP:
					uninit_camera();
					uninit_compass();

					response = "stop received\nstopping daemon\n";
					m = write(newsocketfd, response, strlen(response)); // respond with stopping message
					if (m < 0) {
						uninit_camera();
						uninit_compass();

						syslog(LOG_ERR, "error writing to socket\nexiting...\n");
						if (VERBOSE) printf("error writing to socket\nexiting...\n");
						exit(EXIT_FAILURE);
					} else {
						syslog(LOG_INFO, "stop command received\nstopping daemon...\n");
						if (VERBOSE) printf("stop command received\nstopping daemon...\n");
						exit(EXIT_SUCCESS);
					}
					break;

				case CAPTURE:
					response = "capture received\n";
					m = write(newsocketfd, response, strlen(response));
					if (m < 0) {
						uninit_camera();
						uninit_compass();

						syslog(LOG_ERR, "error writing to socket\nexiting...\n");
						if (VERBOSE) printf("error writing to socket\nexiting...\n");
						exit(EXIT_FAILURE);
					} else {
						syslog(LOG_INFO, "capture %lu us %dx%d command received\n", command.exp_time_us, command.xbin, command.ybin);
						if (VERBOSE) printf("capture %lu us %dx%d command received\n", command.exp_time_us, command.xbin, command.ybin);
					}

					capture_write(&command, &response);

					m = write(newsocketfd, response, strlen(response));
					if (m < 0) {
						uninit_camera();
						uninit_compass();

						syslog(LOG_ERR, "error writing to socket\nexiting...\n");
						if (VERBOSE) printf("error writing to socket\nexiting...\n");
						exit(EXIT_FAILURE);
					} else {
						syslog(LOG_INFO, "file created : %s", response);
						if (VERBOSE) printf("file created : %s", response);
					}
					break;

				default:
					break;
			}

		}


		if (shutdown(newsocketfd, SHUT_RDWR) < 0) {
			uninit_camera();
			uninit_compass();

			syslog(LOG_ERR, "error shutting down connection\nexiting...\n");
			if (VERBOSE) printf("error shutting down connection\nexiting...\n");
			exit(EXIT_FAILURE);
		} else {
			syslog(LOG_INFO, "%d.%d.%d.%d disconnected\n", (address&0xFF), (address&0xFF00)>>8, (address&0xFF0000)>>16, (address&0xFF000000)>>24 );
			if (VERBOSE) printf("%d.%d.%d.%d disconnected\n", (address&0xFF), (address&0xFF00)>>8, (address&0xFF0000)>>16, (address&0xFF000000)>>24 );
		}

		close(newsocketfd);

	} while (loop);

	if (shutdown(socketfd, SHUT_RDWR) < 0) {
		uninit_camera();
		uninit_compass();

		syslog(LOG_ERR, "error shutting down socket\nexiting...\n");
		if (VERBOSE) printf("error shutting down socket\nexiting...\n");
		exit(EXIT_FAILURE);
	} else {
		uninit_camera();
		uninit_compass();

		syslog(LOG_INFO,"camdaemon port %d shutting down...", port);
		if (VERBOSE) printf("camdaemon port %d shutting down...", port);
		exit(EXIT_SUCCESS);
	}

	close(socketfd);

	return 0;
}
