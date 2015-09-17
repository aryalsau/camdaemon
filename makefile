COMLIBS = -lm -ldl -lpthread -lraw1394
PVCAMLIB = -lpvcam
ANDORLIB = -landor
PVCAMDIR = /usr/local/pvcam/examples
IKONDIR = ikonhelper
PIXISDIR = pixishelper
VCAMDIR = camhelper
CFLAGS = -o
TARGET = camdaemon
CC = gcc
RM = rm

#sudo gcc -o camdaemon main.c         -lm -ldl -lpthread -lraw1394 -Ivcam
#sudo gcc -o camdaemon main.c         -lpvcam -lm -ldl -lpthread -lraw1394 -I/usr/local/pvcam/examples -Ipixis
#sudo gcc -o camdaemon main.c -landor -lm -ldl -lpthread -lraw1394 -Iikon

all: $(TARGET)

virtual:
	$(CC) $(CFLAGS) $(TARGET) server.c $(COMLIBS) -I$(VCAMDIR)

pixis:
	$(CC) $(CFLAGS) $(TARGET) server.c $(PVCAMLIB) $(COMLIBS) -I$(PVCAMDIR) -I$(PIXISDIR)

ikon:
	$(CC) $(CFLAGS) $(TARGET) server.c $(ANDORLIB) $(COMLIBS) -I$(IKONDIR)

clear:
	$(RM) $(TARGET)
