ifeq ($(OS),Windows_NT) 
RM = del /Q /F
CP = copy /Y
ifdef ComSpec
SHELL := $(ComSpec)
endif
ifdef COMSPEC
SHELL := $(COMSPEC)
endif
else
RM = rm -rf
CP = cp -f
endif


CC := sdcc
PACKIHX := packihx

.PHONY: all clean

all: timer-8051.hex

clean:
	-$(RM) -f *.asm *.lk *.lst *.map *.mem *.rel *.rst *.sym *.asm *.ihx *.hex

main.ihx: main.c
	$(CC) main.c

timer-8051.hex: main.ihx
	$(PACKIHX) main.ihx > timer-8051.hex

