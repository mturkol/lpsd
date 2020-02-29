CC = gcc
CFLAGS = -Wall -W -O0 -g

BINDIR = /usr/local/bin
CFGDIR = ~

SOURCES = IO.c  ask.c  config.c  debug.c  errors.c lpsd-exec.c \
		  lpsd.c  misc.c tics.c genwin.c ArgParser.c StrParser.c \
		  netlibi0.c goodn.c

OBJECTS = $(SOURCES:.c=.o)

lpsd-exec : $(OBJECTS)
	$(CC) -o lpsd-exec $(OBJECTS) -lm -lfftw3

install:
	cp lpsd-exec $(BINDIR); \
		echo "export LPSDCFN=$(CFGDIR)/lpsd.cfg"> $(BINDIR)/lpsd; \
		cat lpsd >> $(BINDIR)/lpsd
	chmod 755 $(BINDIR)/lpsd

.PHONY: clean install

example-data:
	@echo "Compiling lpsd_test and creating example file test.dat."
	make -C example/
	@echo "Running lpsd with test file..."
	# ./lpsd-exec --input=example/test.dat

clean-example-data:
	make -C example/ clean

clean:
	rm $(OBJECTS) lpsd-exec


