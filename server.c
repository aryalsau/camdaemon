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

void processCommand(char command[], int sock);
static void socketHook(int sock);
static void signalHandler(int signum);
static char * parseIpAddress(int address);
static void exitError(const char *msg);
static char * stopDaemon(int sock);
static char * restartDaemon();

int verbose = 0;
int loop = 1;

static void exitError(const char *msg) {
  int errsv = errno;
  if (errno){
    perror(msg);
  } else {
    printf("%s\n",msg);
  }
  exit(EXIT_FAILURE);
}

static void signalHandler(int signum) {
	switch ( signum ) {
		case SIGKILL:
			syslog( LOG_INFO, "SIGKILL Received... \n");
      if (verbose)
        printf("SIGKILL Received... \n");
			exit(EXIT_SUCCESS);
			break;
		case SIGTERM:
			syslog( LOG_INFO, "SIGTERM Received... \n");
      if (verbose)
        printf("SIGTERM Received... \n");
			exit(EXIT_SUCCESS);
			break;
		case SIGUSR1:
			syslog( LOG_INFO, "SIGUSR1 Received... \n");
      if (verbose)
        printf("SIGUSR1 Received... \n");
			exit(EXIT_SUCCESS);
			break;
		case SIGUSR2:
			syslog( LOG_INFO, "SIGUSR2 Received... \n");
      if (verbose)
        printf("SIGUSR2 Received... \n");
			exit(EXIT_SUCCESS);
			break;
		case SIGSEGV:
			syslog( LOG_INFO, "SIGSEGV Received... \n");
      if (verbose)
        printf("SIGSEGV Received... \n");
			exit(EXIT_SUCCESS);
			break;
	}
}

static char * parseIpAddress(int address) {
	static char addressBuffer[80];
	sprintf(addressBuffer, "%d.%d.%d.%d", (address&0xFF), ((address&0xFF00)>>8), ((address&0xFF0000)>>16), ((address&0xFF000000)>>24));
	return addressBuffer;
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

void processCommand(char command[], int sock) {
	static const char previewComm[] = "preview";
	static const char captureComm[] = "capture";
	static const char stopdComm[] = "stopd";

	char * commBuffer[5];
	int commSize;

	int n;

	char * commandResponse;

	if (strstr(command,captureComm) != NULL){
		commSize = split(command,commBuffer);

		commandResponse = capture(strtol(commBuffer[1],NULL,0));

    syslog( LOG_INFO, "capture command received. writing file : %s\n",commandResponse);
    if (verbose)
      printf("capture command received. writing file : %s\n",commandResponse);

		n = write(sock,commandResponse,strlen(commandResponse));
		if (n < 0) {
			perror("ERROR writing to socket");
			exit(1);
		}

	} else if (strstr(command,previewComm) != NULL){
		commSize = split(command,commBuffer);

    syslog( LOG_INFO, "preview command received. sending data...\n");
    if (verbose)
      printf("preview command received. sending data...\n");

		commandResponse = preview(strtol(commBuffer[1],NULL,0),sock);

	} else if (strstr(command,stopdComm) != NULL){
		commSize = split(command,commBuffer);

    syslog( LOG_INFO, "stopd command received. stopping camdaemon...\n");
    if (verbose)
      printf("stopd command received. stopping camdaemon...\n");

		commandResponse = stopDaemon(sock);

		n = write(sock,commandResponse,strlen(commandResponse));
		if (n < 0) {
			perror("ERROR writing to socket");
			exit(1);
		}

	} else {

    syslog( LOG_INFO, "Invalid command received.\n");
    if (verbose)
      printf("Invalid command received.\n");

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

	char rxBuffer[256];
	char commBuffer[256];

	bzero(rxBuffer,256);
	bzero(commBuffer,256);
	n = read(sock,rxBuffer,255);
	memcpy(commBuffer, rxBuffer, strlen(rxBuffer)+1);
	bzero(rxBuffer,256);

	if (n < 0)
		exitError("ERROR reading from socket");

	processCommand(commBuffer,sock);

	bzero(commBuffer,256);

}

static char * stopDaemon(int sock) {
	char * stopResponse;
	stopResponse = "stopping camdaemon";
	uninitCamera();
  loop = 0;
	return stopResponse;
}

void printUsage() {
    printf("Usage: camdaemon -v\n\tv - verbose\n");
}

int main(int argc , char *argv[]) {

  int option = 0;
  char errMsgBuffer[80];
  int iSetOption = 1;

  sleep(1);

  int portno = 8000;

  while ((option = getopt(argc, argv,"vp:")) != -1) {
    switch (option) {
      case 'v' :
        verbose = 1;
        break;
      case 'p' :
        portno = atoi(optarg);
        break;
      default:
        printUsage();
        sprintf(errMsgBuffer, "Unknown Option : %s", argv[1]);
        exitError(errMsgBuffer);
    }
  }


  signal(SIGUSR1, signalHandler);
  signal(SIGUSR2, signalHandler);
  signal(SIGTERM, signalHandler);
  signal(SIGKILL, signalHandler);
  signal(SIGSEGV, signalHandler);


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
    exitError("ERROR opening socket");
    exit(EXIT_FAILURE);
  } else {
    syslog(LOG_INFO,"camdaemon started on port %d", portno);
    if (verbose)
      printf("camdaemon started on port %d\n",portno);
  }

  bzero((char *) &server, sizeof(server));

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(portno);
  setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, (char*)&iSetOption, sizeof(iSetOption));

  if (bind(socketfd, (struct sockaddr *) &server,sizeof(server)) < 0) {
    syslog(LOG_INFO,"ERROR on binding to port %d", portno);
    if (verbose)
      printf("ERROR on binding to port %d\n",portno);
    exitError("ERROR on binding");
    exit(EXIT_FAILURE);
  }



  initCamera();

	listen(socketfd,5);
  do {

		clientlen = sizeof(client);
		newsocketfd = accept(socketfd, (struct sockaddr *) &client,  &clientlen);
		if (newsocketfd < 0) {
			exitError("ERROR on accept");
			exit(EXIT_FAILURE);
		}

		syslog(LOG_INFO,"%s connected", parseIpAddress(client.sin_addr.s_addr));
    if (verbose)
      printf("%s connected\n", parseIpAddress(client.sin_addr.s_addr));

		socketHook(newsocketfd);

    if (shutdown(newsocketfd, SHUT_RDWR) < 0) {
      exitError("ERROR shutting down connection");
      exit(EXIT_FAILURE);
    } else {
      syslog(LOG_INFO,"%s disconnected", parseIpAddress(client.sin_addr.s_addr));
      if (verbose)
        printf("%s disconnected\n", parseIpAddress(client.sin_addr.s_addr));
    }
		close(newsocketfd);

	} while (loop);






  if (shutdown(socketfd, SHUT_RDWR) < 0) {
    exitError("ERROR shutting down socket");
    exit(EXIT_FAILURE);
  } else {
    syslog(LOG_INFO,"camdaemon port %d shutting down", portno);
    if (verbose)
      printf("camdaemon port %d shutting down\n", portno);
  }
  close(socketfd);
	return 0;
}
