COMLIBS = -lm -ldl -lpthread -lraw1394 -lcfitsio -lnsl -lphidget21
PVCAMLIB = -lpvcam
ANDORLIB = -landor
PVCAMDIR = /usr/local/pvcam/examples
IKONDIR = ./ikonhelper
PIXISDIR = ./pixishelper
VCAMDIR = ./camhelper
CFLAGS = -o
TARGET = camdaemon
CC = gcc
RM = rm

#sudo gcc -o camdaemon server.c         -lm -ldl -lpthread -lraw1394 -Ivcam
#sudo gcc -o camdaemon server.c         -lpvcam -lm -ldl -lpthread -lraw1394 -I/usr/local/pvcam/examples -Ipixis
#sudo gcc -o camdaemon server.c -landor -lm -ldl -lpthread -lraw1394 -Iikon

clean:
	$(RM) $(TARGET)
	$(RM) camhelper.o

all: $(TARGET)

virtual:
	$(CC) $(CFLAGS) camhelper.o -c $(VCAMDIR)/camhelper.c -I$(VCAMDIR)
	$(CC) camhelper.o $(CFLAGS) $(TARGET) server.c $(COMLIBS) -I$(VCAMDIR)

pixis:
	$(CC) $(CFLAGS) $(TARGET) server.c $(PVCAMLIB) $(COMLIBS) -I$(PVCAMDIR) -I$(PIXISDIR)

ikon:
	$(CC) $(CFLAGS) $(TARGET) server.c $(ANDORLIB) $(COMLIBS) -I$(IKONDIR)
