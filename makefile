COMLIBS = -lm -ldl -lpthread -lraw1394
PVCAMLIB = -lpvcam
ANDORLIB = -landor
IDIR = /usr/local/pvcam/examples
INCLUDES = -I$(IDIR)
CFLAGS = -o
TARGET = camdaemon
CC = gcc
RM = rm

#gcc -o camdaemon main.c -lpvcam -lm -ldl -lpthread -lraw1394 -I/usr/local/pvcam/examples
#gcc -o camdaemon main.c -landor -lpvcam -lm -ldl -lpthread -lraw1394 -I/usr/local/pvcam/examples

all: $(TARGET)

virtual:
	$(CC) $(CFLAGS) $(TARGET) main.c $(COMLIBS) $(INCLUDES)

pixis:
	$(CC) $(CFLAGS) $(TARGET) main.c $(PVCAMLIB) $(COMLIBS) $(INCLUDES)

andor:
	$(CC) $(CFLAGS) $(TARGET) main.c $(ANDORLIB) $(COMLIBS) $(INCLUDES)

clear:
	$(RM) $(TARGET)