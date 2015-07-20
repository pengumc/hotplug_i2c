#author: michiel van der Coelen

NAME = i2ccontrol
EXT = .exe
CC = gcc
CFLAGS = -std=c11 -I. -Iinclude -Wall -Wconversion 
LIBS = -Lbin -lhidapi
OBJECTS = $(NAME).o
#`pkg-config libusb-1.0 --cflags`
#`pkg-config libusb-1.0 --libs`
ifeq ($(shell uname), Linux)
	CFLAGS = -I. -Wall -Wconversion 
	LIBS = -L. -lhidapi-libusb -lusb-1.0
	EXT = 
endif

all:bin/$(NAME)$(EXT)


bin/$(NAME)$(EXT):$(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LIBS) -o $@

%.o:src/%.c
	$(CC) $(CFLAGS) $< -c  -o $@

clean:
	rm -f $(OBJECTS)
