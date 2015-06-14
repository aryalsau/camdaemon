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

pixis:
	$(CC) $(CFLAGS) $(TARGET) main.c $(COMLIBS) $(PVCAMLIB) $(INCLUDES)

andor:
	$(CC) $(CFLAGS) $(TARGET) main.c $(COMLIBS) $(ANDORLIB) $(PVCAMLIB) $(INCLUDES)

clear:
	$(RM) $(TARGET)