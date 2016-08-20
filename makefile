CFLAGS = -Wall -Werror -Wmissing-prototypes -Wmissing-declarations -pedantic -std=iso9899:1999 -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wstrict-prototypes

COMLIBS = -lm -ldl -lpthread -lraw1394 -lcfitsio -lnsl

NULCAMSRC = camera/nullcam.c
NULLCOMPASSSRC = compass/nullcompass.c

PHIDGETSRC = compass/phidget.c
PHIDGETLIB = -lphidget21

PIXISSRC = camera/pixis.c
PVCAMLIB = -lpvcam
PVCAMDIR = /usr/local/pvcam/examples

IKONSRC = camera/ikon.c
ANDORLIB = -landor

TARGET = camdaemon
CC = gcc
RM = rm

clean :
	@echo -cleaning-
	$(RM) -f $(TARGET)
	$(RM) -f *.o
	@echo -clean complete-

$(TARGET) : camera.o compass.o server.c common.c
	@echo -building camdaemon-
	$(CC) $(CFLAGS) -o $(TARGET) server.c common.c $(COMLIBS) camera.o compass.o $(PVCAMLIB) $(ANDORLIB) $(PHIDGETLIB)
	@echo -build complete-

nullcam : $(NULCAMSRC)
	@echo -building nullcam camera object-
	$(CC) $(CFLAGS) -c $(NULCAMSRC) -o camera.o
	@echo -build complete-

pixis : $(PIXISSRC)
	@echo -building pixis camera object-
	$(CC) -Wall -Wmissing-prototypes -Wmissing-declarations -pedantic -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wstrict-prototypes -c $(PIXISSRC) -o camera.o $(PVCAMLIB) -I$(PVCAMDIR)
	@echo -build complete-

ikon : $(IKONSRC)
	@echo -building ikon camera object-
	$(CC) $(CFLAGS) -c $(IKONSRC) -o camera.o $(ANDORLIB)
	@echo -build complete-

phidget : $(PHIDGETSRC)
	@echo -building compass object-
	$(CC) $(CFLAGS) -c $(PHIDGETSRC) -o compass.o $(PHIDGETLIB)
	@echo -build complete-

nullcompass : $(NULLCOMPASSSRC)
	@echo -building compass object-
	$(CC) $(CFLAGS) -c $(NULLCOMPASSSRC) -o compass.o
	@echo -build complete-
