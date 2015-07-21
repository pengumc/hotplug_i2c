# Makefile

MMCU = atmega88
AVRDUDE_MCU = m88
AVRDUDE_PROGRAMMERID = usbasp
F_CPU = 20000000
NAME = i2cdebug
OBJECTS = usbdrv.o usbdrvasm.o i2cmaster.o $(NAME).o
CFLAGS = -DF_CPU=$(F_CPU) -std=c99 -Wall -Os -mmcu=$(MMCU) -Iinclude -I.
CC = avr-gcc
SIZE = avr-size
OBJCOPY = avr-objcopy

all: bin/$(NAME).hex
	$(SIZE) bin/$(NAME).hex

bin/$(NAME).hex: $(NAME).elf
	$(OBJCOPY) -O ihex $< $@
	
$(NAME).elf: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)
	
%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: src/%.S
	$(CC) $(CFLAGS) -x assembler-with-cpp -c $< -o $@

clean:
	rm -f $(OBJECTS) $(NAME).elf
	
program: bin/$(NAME).hex
	avrdude -c $(AVRDUDE_PROGRAMMERID) -p $(AVRDUDE_MCU) -U flash:w:bin/$(NAME).hex

test:
	avrdude -c $(AVRDUDE_PROGRAMMERID) -p $(AVRDUDE_MCU) -v

#set fuse: -U [l/h/e]fuse:w:0x##:m 