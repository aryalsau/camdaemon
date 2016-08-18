COMLIBS = -lm -ldl -lpthread -lraw1394 -lcfitsio -lnsl -lphidget21
VIRTUALSRC = camera/virtual.c
PIXISSRC = camera/pixis.c
IKONSRC = camera/ikon.c
COMPASSSRC = compass/compass.c
PVCAMLIB = -lpvcam
ANDORLIB = -landor
PVCAMDIR = /usr/local/pvcam/examples
CFLAGS = -Wall -Werror -Wmissing-prototypes -Wmissing-declarations -pedantic -std=iso9899:1999 -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wstrict-prototypes
TARGET = camdaemon
CC = gcc
RM = rm


clean :
	@echo -cleaning-
	$(RM) -f $(TARGET)
	$(RM) -f *.o
	@echo -clean complete-


virtual : virtual.o compass.o server.c common.c
	@echo -building virtual camdaemon-
	$(CC) $(CFLAGS) -o $(TARGET) server.c common.c $(COMLIBS) virtual.o compass.o
	@echo -build complete-

virtual.o : $(VIRTUALSRC)
	@echo -building virtual camera object-
	$(CC) $(CFLAGS) -c $(VIRTUALSRC) -o virtual.o
	@echo -build complete-


pixis : pixis.o compass.o server.c common.c
	@echo -building pixis camdaemon-
	$(CC) -std=c99 -Wall -Wmissing-prototypes -Wmissing-declarations -pedantic -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wstrict-prototypes -o $(TARGET) server.c common.c $(PVCAMLIB) $(COMLIBS) -I$(PVCAMDIR) pixis.o compass.o
	@echo -build complete-

pixis.o : $(PIXISSRC)
	@echo -building pixis camera object-
	$(CC) -Wall -Wmissing-prototypes -Wmissing-declarations -pedantic -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wstrict-prototypes -c $(PIXISSRC) -o pixis.o $(PVCAMLIB) -I$(PVCAMDIR)
	@echo -build complete-


ikon : ikon.o compass.o server.c common.c
	@echo -building ikon camdaemon-
	$(CC) $(CFLAGS) -o $(TARGET) server.c common.c $(ANDORLIB) $(COMLIBS) ikon.o compass.o
	@echo -build complete-

ikon.o : $(IKONSRC)
	@echo -building ikon camera object-
	$(CC) $(CFLAGS) -c $(IKONSRC) -o ikon.o $(ANDORLIB)
	@echo -build complete-


compass.o : $(COMPASSSRC)
	@echo -building compass object-
	$(CC) $(CFLAGS) -c $(COMPASSSRC) -o compass.o
	@echo -build complete-
