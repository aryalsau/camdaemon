COMLIBS = -lm -ldl -lpthread -lraw1394
PVCAMLIB = -lpvcam
ANDORLIB = -landor
COMDIR = /usr/local/pvcam/examples
IKONDIR = ikonhelper
PIXISDIR = pixishelper
VCAMDIR = camhelper
CFLAGS = -o
TARGET = camdaemon
CC = gcc
RM = rm

#sudo gcc -o camdaemon main.c         -lpvcam -lm -ldl -lpthread -lraw1394 -I/usr/local/pvcam/examples -Ivcam
#sudo gcc -o camdaemon main.c         -lpvcam -lm -ldl -lpthread -lraw1394 -I/usr/local/pvcam/examples -Ipixis
#sudo gcc -o camdaemon main.c -landor -lpvcam -lm -ldl -lpthread -lraw1394 -I/usr/local/pvcam/examples -Iikon

all: $(TARGET)

virtual:
	$(CC) $(CFLAGS) $(TARGET) server.c $(PVCAMLIB) $(COMLIBS) -I$(COMDIR) -I$(VCAMDIR)

pixis:
	$(CC) $(CFLAGS) $(TARGET) main.c $(PVCAMLIB) $(COMLIBS) -I$(COMDIR) -I$(PIXISDIR)

ikon:
	$(CC) $(CFLAGS) $(TARGET) main.c $(ANDORLIB) $(PVCAMLIB) $(COMLIBS) -I$(COMDIR) -I$(IKONDIR)

clear:
	$(RM) $(TARGET)
