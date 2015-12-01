//sudo gcc -o camdaemon main.c -landor -lpvcam -lm -ldl -lpthread -lraw1394 -I/usr/local/pvcam/examples
//gcc -o camdaemon main.c -lpvcam -lm -ldl -lpthread -lraw1394 -I/usr/local/pvcam/examples
//top -p $(pgrep -d',' camdaemon)
//tail -f /var/log/syslog
#ifdef __GNUC__
#  if(__GNUC__ > 3 || __GNUC__ ==3)
#	define _GNUC3_
#  endif
#endif

#include <sys/socket.h>
#include <netinet/in.h>

#include "camhelper.c"

void process_command(char command[], int socket);
static void socket_hook(int socket);
static void signal_handler(int signum);
static char * parse_ip_address(int address);
static void exit_error(const char *msg);
static char * stop_daemon(int socket);
static char * restart_daemon();

int verbose = 0;
int loop = 1;

static void exit_error(const char *msg) {
	int errsv = errno;
	if (errno){
		perror(msg);
	} else {
		printf("%s\n",msg);
	}
	exit(EXIT_FAILURE);
}

static void signal_handler(int signum) {
	switch ( signum ) {
		case SIGKILL:
			syslog( LOG_INFO, "SIGKILL Received... \n");
			if (verbose) printf("SIGKILL Received... \n");
			exit(EXIT_SUCCESS);
			break;
		case SIGTERM:
			syslog( LOG_INFO, "SIGTERM Received... \n");
			if (verbose) printf("SIGTERM Received... \n");
			exit(EXIT_SUCCESS);
			break;
		case SIGUSR1:
			syslog( LOG_INFO, "SIGUSR1 Received... \n");
			if (verbose) printf("SIGUSR1 Received... \n");
			exit(EXIT_SUCCESS);
			break;
		case SIGUSR2:
			syslog( LOG_INFO, "SIGUSR2 Received... \n");
			if (verbose) printf("SIGUSR2 Received... \n");
			exit(EXIT_SUCCESS);
			break;
		case SIGSEGV:
			syslog( LOG_INFO, "SIGSEGV Received... \n");
			if (verbose) printf("SIGSEGV Received... \n");
			exit(EXIT_SUCCESS);
			break;
	}
}

static char * parse_ip_address(int address) {
	static char address_buffer[80];
	sprintf(address_buffer, "%d.%d.%d.%d", (address&0xFF), ((address&0xFF00)>>8), ((address&0xFF0000)>>16), ((address&0xFF000000)>>24));
	return address_buffer;
}

static int split(char str[], char * str_array[]) {
	char * pch;
	pch = strtok (str," ,.-");
	int i = 0;
	while (pch != NULL) {
		str_array[i] = pch;
		pch = strtok (NULL, " ,.-");
		i++;
	}
	return i;
}

void process_command(char command[], int socket) {
	static const char capture_command[] = "capture";
	static const char stopd_command[] = "stopd";

	char * command_buffer[5];
	int command_size;

	int n;

	char * response;

	if (strstr(command, capture_command) != NULL){ // capture command received

		command_size = split(command,command_buffer); // split the exposure time

		response = capture_write(strtol(command_buffer[1],NULL,0));

		syslog( LOG_INFO, "capture command received.");
		syslog( LOG_INFO, "writing file : %s",response);

		if (verbose)	printf("capture command received.\nwriting file : %s\n",response);

		n = write(socket,response,strlen(response)); // respond with the filename response
		if (n < 0) {
			perror("ERROR writing to socket");
			exit(1);
		}

	}	else if (strstr(command,stopd_command) != NULL){ // stop command received

		command_size = split(command,command_buffer);

		syslog( LOG_INFO, "stopd command received.");
		syslog( LOG_INFO, "stopping camdaemon...\n");
		if (verbose) printf("stopd command received.\nstopping camdaemon...\n");

		response = stop_daemon(socket);

		n = write(socket,response,strlen(response)); // respond with the stopping message
		if (n < 0) {
			perror("ERROR writing to socket");
			exit(1);
		}

	} else { // invalid command received

    syslog( LOG_INFO, "Invalid command received.\n");
    if (verbose)
      printf("Invalid command received.\n");

		response = "invalid command";

		n = write(socket,response,strlen(response)); // respond with the invalid command message
		if (n < 0) {
			perror("ERROR writing to socket");
			exit(1);
		}

	}

}

static void socket_hook(int socket) {

	int n;

	unsigned short * response;

	char rx_buffer[256];
	char command_buffer[256];

	bzero(rx_buffer,256);
	bzero(command_buffer,256);
	n = read(socket,rx_buffer,255);
	memcpy(command_buffer, rx_buffer, strlen(rx_buffer)+1);
	bzero(rx_buffer,256);

	if (n < 0) exit_error("ERROR reading from socket");

	process_command(command_buffer,socket);

	bzero(command_buffer,256);

}

static char * stop_daemon(int socket) {
	char * stop_response;
	stop_response = "stopping camdaemon";
	uninit_camera();
	loop = 0;
	return stop_response;
}

void print_usage() {
	printf("Usage: camdaemon -v\n\tv - verbose\n");
}

int main(int argc , char *argv[]) {

	int option = 0;
	char error_buffer[80];
	int iset_option = 1;

	sleep(1);

  int port = 8000;

	while ((option = getopt(argc, argv,"vp:")) != -1) {
		switch (option) {
		case 'v' :
			verbose = 1;
			break;
		case 'p' :
			port = atoi(optarg);
			break;
		default:
			print_usage();
			sprintf(error_buffer, "Unknown Option : %s", argv[1]);
			exit_error(error_buffer);
		}
	}


	signal(SIGUSR1, signal_handler);
	signal(SIGUSR2, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGKILL, signal_handler);
	signal(SIGSEGV, signal_handler);


	/* here is the socket */
	int socketfd, newsocketfd;
	socklen_t clientlen;
	struct sockaddr_in server, client;
	int n, m;





































  /* Daemon-specific initialization goes here */

  /* here is more socket stuff */
  socketfd = socket(AF_INET, SOCK_STREAM, 0);
  // fcntl(socketfd, F_SETFL, O_NONBLOCK);
	if (socketfd < 0) {
		exit_error("ERROR opening socket");
		exit(EXIT_FAILURE);
	} else {
		syslog(LOG_INFO,"camdaemon started on port %d", port);
		if (verbose) printf("camdaemon started on port %d\n",port);
	}

	bzero((char *) &server, sizeof(server));

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(port);
	setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, (char*)&iset_option, sizeof(iset_option));

  if (bind(socketfd, (struct sockaddr *) &server,sizeof(server)) < 0) {
		syslog(LOG_INFO,"ERROR on binding to port %d", port);
		if (verbose) printf("ERROR on binding to port %d\n",port);
		exit_error("ERROR on binding");
		exit(EXIT_FAILURE);
	}



  init_camera();

	listen(socketfd,5);
  do {

		clientlen = sizeof(client);
		newsocketfd = accept(socketfd, (struct sockaddr *) &client,  &clientlen);
		if (newsocketfd < 0) {
			exit_error("ERROR on accept");
			exit(EXIT_FAILURE);
		}

		syslog(LOG_INFO,"%s connected", parse_ip_address(client.sin_addr.s_addr));
		if (verbose) printf("%s connected\n", parse_ip_address(client.sin_addr.s_addr));

		socket_hook(newsocketfd);

		if (shutdown(newsocketfd, SHUT_RDWR) < 0) {
		  exit_error("ERROR shutting down connection");
		  exit(EXIT_FAILURE);
		} else {
			syslog(LOG_INFO,"%s disconnected", parse_ip_address(client.sin_addr.s_addr));
			if (verbose) printf("%s disconnected\n", parse_ip_address(client.sin_addr.s_addr));
		}
		close(newsocketfd);

	} while (loop);






	if (shutdown(socketfd, SHUT_RDWR) < 0) {
		exit_error("ERROR shutting down socket");
		exit(EXIT_FAILURE);
	} else {
		syslog(LOG_INFO,"camdaemon port %d shutting down", port);
		if (verbose) printf("camdaemon port %d shutting down\n", port);
	}
	close(socketfd);
	return 0;
}
