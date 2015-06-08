//sudo gcc -o camdaemon main.c -landor -lpvcam -lm -ldl -lpthread -lraw1394 -I/usr/local/pvcam/examples

//top -p $(pgrep -d',' name)
//tail -f /var/log/syslog to top by name
#ifdef __GNUC__
#  if(__GNUC__ > 3 || __GNUC__ ==3)
#	define _GNUC3_
#  endif
#endif

#include <sys/socket.h>
#include <netinet/in.h>

#include "pixishelper.c"
// #include "camhelper.c"

// char *cam = getenv("HITANDMIS_CAM");
// if(cam) {
// 	if(strcmp(cam, "ikon") == 0)  {
// 		#include "ikonhelpe.c"
// 	} else if(strcmp(cam, "pixis") == 0){
// 		#include "pixishelper.c"
// 	} else if(strcmp(cam, "vcam") == 0){
// 		#include "camhelper.c"
// 	}
// }


//static char * processCommand(char command[]);
void processCommand(char command[], int sock);
static void socketHook(int sock);
static void signalHandler(int signum);
static char * parseIpAddress(int address);
static void exitError(const char *msg);
static char * stopDaemon();
static char * restartDaemon();

static void exitError(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

static void signalHandler(int signum) {
	switch ( signum ) {
		case SIGKILL:
			syslog( LOG_INFO, "SIGKILL Received... \n");
			exit(EXIT_SUCCESS);
			break;
		case SIGTERM:
			syslog( LOG_INFO, "SIGTERM Received... \n");
			exit(EXIT_SUCCESS);
			break;
		case SIGUSR1:
			syslog( LOG_INFO, "SIGUSR1 Received... \n");
			exit(EXIT_SUCCESS);
			break;
		case SIGUSR2:
			syslog( LOG_INFO, "SIGUSR2 Received... \n");
			exit(EXIT_SUCCESS);
			break;
		case SIGSEGV:
			syslog( LOG_INFO, "SIGSEGV Received... \n");
			exit(EXIT_SUCCESS);
			break;
	}
}

static char * parseIpAddress(int address) {
	static char address_bufer[80];
	sprintf(address_bufer, "%d.%d.%d.%d", (address&0xFF), ((address&0xFF00)>>8), ((address&0xFF0000)>>16), ((address&0xFF000000)>>24));
	return address_bufer;
}

static int split(char str[], char * strArray[]) {
	char * pch;
	pch = strtok (str," ,.-");
	int i = 0;
	while (pch != NULL) {
		strArray[i] = pch;
		pch = strtok (NULL, " ,.-");
		i++;
	}
	return i;
}

// static char * processCommand(char command[]) {
// 	static const char pcaptureComm[] = "preview";
// 	static const char captureComm[] = "capture";
// 	static const char stopdComm[] = "stopd";

// 	char * commBuffer[5];
// 	int commSize;

// 	char * commandResponse;

// 	if (strstr(command,captureComm) != NULL){
// 		commSize = split(command,commBuffer);
// 		commandResponse = capture(strtol(commBuffer[1],NULL,0));
// 	} else if (strstr(command,pcaptureComm) != NULL){
// 		commSize = split(command,commBuffer);
// 		commandResponse = preview(strtol(commBuffer[1],NULL,0));
// 	} else if (strstr(command,stopdComm) != NULL){
// 		commSize = split(command,commBuffer);
// 		commandResponse = stopDaemon();
// 	} else {
// 		commandResponse = "invalid command";
// 	}

// 	return commandResponse;

// }

// static void socketHook(int sock) {

// 	int n;

// 	char * response;

// 	char rx_buffer[256];
// 	char comm_buffer[256];
// 	char tx_buffer[256];

// 	bzero(rx_buffer,256);
// 	bzero(comm_buffer,256);
// 	n = read(sock,rx_buffer,255);
// 	memcpy(comm_buffer, rx_buffer, strlen(rx_buffer)+1);
// 	bzero(rx_buffer,256);

// 	if (n < 0)
// 		exitError("ERROR reading from socket");

// 	response = processCommand(comm_buffer);
// 	memcpy(tx_buffer, response, strlen(response)+1);
	
// 	n = write(sock,tx_buffer,strlen(response));
// 	if (n < 0) {
// 		perror("ERROR writing to socket");
// 		exit(1);
// 	}

// 	bzero(comm_buffer,256);
// }


void processCommand(char command[], int sock) {
	static const char pcaptureComm[] = "preview";
	static const char captureComm[] = "capture";
	static const char stopdComm[] = "stopd";

	char * commBuffer[5];
	int commSize;

	int n;

	char * commandResponse;

	if (strstr(command,captureComm) != NULL){
		commSize = split(command,commBuffer);
		commandResponse = capture(strtol(commBuffer[1],NULL,0));

		n = write(sock,commandResponse,strlen(commandResponse));
		if (n < 0) {
			perror("ERROR writing to socket");
			exit(1);
		}

	} else if (strstr(command,pcaptureComm) != NULL){
		commSize = split(command,commBuffer);
		commandResponse = preview(strtol(commBuffer[1],NULL,0),sock);

	} else if (strstr(command,stopdComm) != NULL){
		commSize = split(command,commBuffer);
		commandResponse = stopDaemon();

		n = write(sock,commandResponse,strlen(commandResponse));
		if (n < 0) {
			perror("ERROR writing to socket");
			exit(1);
		}

	} else {
		commandResponse = "invalid command";

		n = write(sock,commandResponse,strlen(commandResponse));
		if (n < 0) {
			perror("ERROR writing to socket");
			exit(1);
		}

	}


}

static void socketHook(int sock) {

	int n;

	uns16 * response;

	char rx_buffer[256];
	char comm_buffer[256];

	bzero(rx_buffer,256);
	bzero(comm_buffer,256);
	n = read(sock,rx_buffer,255);
	memcpy(comm_buffer, rx_buffer, strlen(rx_buffer)+1);
	bzero(rx_buffer,256);

	if (n < 0)
		exitError("ERROR reading from socket");

	processCommand(comm_buffer,sock);

	bzero(comm_buffer,256);
	
}

static char * stopDaemon() {
	char * stopResponse;
	stopResponse = "stopping camdaemon";
	syslog( LOG_INFO, "stopd Received. Stopping camdaemon...\n");
	uninitCamera();
	exit(EXIT_SUCCESS);
	return stopResponse;
}

int main(int argc, char *argv[]) {

	signal(SIGUSR1, signalHandler);
	signal(SIGUSR2, signalHandler);
	signal(SIGTERM, signalHandler);
	signal(SIGKILL, signalHandler);
	signal(SIGSEGV, signalHandler);
		
	/* Our process ID and Session ID */
	pid_t pid, sid;

	/* here is the socket */
	int socketfd, newsocketfd;
	int portno;
	socklen_t clientlen;
	struct sockaddr_in server, client;
	int n, m;
	
	/* Fork off the parent process */
	pid = fork();
	if (pid < 0) { /* Log the failure */
			exit(EXIT_FAILURE);
	} else if (pid > 0) { /* If we got a good PID, then we can exit the parent process. */
			exit(EXIT_SUCCESS);
	}

	/* Change the file mode mask */
	umask(0);
			
	/* Open any logs here */        

	char *portChar;
	/* Create a new SID for the child process */
	sid = setsid();
	if (sid < 0) { /* Log the failure */
			exit(EXIT_FAILURE);
	} else {
		 if (argc < 2) {
			 //exitError("ERROR, no port provided");
			 //exit(EXIT_FAILURE);
		 	
		 	portChar = getenv("HITANDMIS_CAMDAEMON_PORT");
		 	syslog(LOG_INFO,"no port provided, using %s", portChar);
		 } else {
		 	portChar = argv[1];
		 }
	}
	
	/* Change the current working directory */
	if ((chdir("/")) < 0) { /* Log the failure */
			exit(EXIT_FAILURE);
	}
	
	/* Close out the standard file descriptors */
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	
	/* Daemon-specific initialization goes here */

	/* here is more socket stuff */
	socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if (socketfd < 0) {
		exitError("ERROR opening socket");
		exit(EXIT_FAILURE);
	}

	bzero((char *) &server, sizeof(server));
	portno = atoi(portChar);
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(portno);
	if (bind(socketfd, (struct sockaddr *) &server,sizeof(server)) < 0) {
		exitError("ERROR on binding");
		exit(EXIT_FAILURE);
	}

	initCamera();

	listen(socketfd,5);
	while (1) {
		clientlen = sizeof(client);
		newsocketfd = accept(socketfd, (struct sockaddr *) &client,  &clientlen);
		if (newsocketfd < 0) {
			exitError("ERROR on accept");
			exit(EXIT_FAILURE);
		}

		syslog(LOG_INFO,"%s connected", parseIpAddress(client.sin_addr.s_addr));

		socketHook(newsocketfd);
		close(newsocketfd);

	}
	exit(EXIT_SUCCESS);
}