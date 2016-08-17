COMLIBS = -lm -ldl -lpthread -lraw1394 -lcfitsio -lnsl -lphidget21
VIRTUALSRC = virtual.c
COMPASSSRC = compass.c
CFLAGS = -Wall -Werror -Wmissing-prototypes -Wmissing-declarations -pedantic -std=iso9899:1999 -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wstrict-prototypes
TARGET = camdaemon
CC = gcc
RM = rm
OBJ = camera.o compass.o

clean :
	@echo -cleaning-
	-rm -f $(TARGET)
	-rm -f $(OBJ)
	@echo -clean complete-

all: $(TARGET)

virtual: camera.o compass.o server.c common.c
	$(CC) $(CFLAGS) -o $(TARGET) server.c common.c $(COMLIBS) -I$(VIRTUALSRC) $(OBJ)

camera.o: camera/virtual.c
	$(CC) $(CFLAGS) -c camera/virtual.c -o camera.o

compass.o: compass/compass.c
	$(CC) $(CFLAGS) -c compass/compass.c -o compass.o
