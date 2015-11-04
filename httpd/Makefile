ifndef CROSS_CC_PREFIX
	CROSS_CC_PREFIX=$(CROSS_COMPILE)
endif

CC=$(CROSS_CC_PREFIX)gcc
LD=$(CROSS_CC_PREFIX)ld

CFLAGS = -Wall

SRC = granelver.c pwned.html
OBJ = granelver.o pwned.o

%.o: %.html
	$(LD) -r -b binary -o $@ $<

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

granelver: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

all: granelver

clean:
	rm -f *.o granelver
