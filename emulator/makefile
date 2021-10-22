ifeq ($(OS),Windows_NT)
include ..\documents\common.make
else
include ../documents/common.make
endif

APPNAME = acenext$(APPSTEM)

SOURCES = 	src$(S)sys_processor.o  framework$(S)main.o framework$(S)gfx.o framework$(S)debugger.o \
			src$(S)sys_debug_ace.o src$(S)hardware.o src$(S)hardware_emu.o src$(S)hwtape.o
  
CC = g++

.PHONY: all clean emulator test prebuild milli esp32

all: emulator
#	.$(S)$(APPNAME) 

esp32:
	make -C ..$(S)esp32

run: emulator 
	.$(S)$(APPNAME) xxx go

milli: emulator 
	.$(S)$(APPNAME) spool$(S)milli.spl go

bench: emulator 
	.$(S)$(APPNAME) spool$(S)bench.spl go

debug: emulator 
	.$(S)$(APPNAME) 
	
test : emulator
	$(CDEL) memory.dump $(CDELQ)
	zasm -u test.asm
	.$(S)$(APPNAME) test.rom

emulator: prebuild $(APPNAME)
#	$(CCOPY) $(APPNAME) ..$(S)..$(S)bin$(S) $(CCOPYQ)

prebuild:
	make -C ..$(S)z80
	make -C ..$(S)rom
	$(CCOPY) ..$(S)z80$(S)code$(S)*.h cpu
	$(CCOPY) ..$(S)rom$(S)*.h roms

%.o:%.cpp
	$(CC) $(CXXFLAGS) -D INCLUDE_DEBUGGING_SUPPORT -I cpu -I framework -I include -I roms -c -o $@ $<

clean:
	$(CDEL) $(APPNAME) 
	$(CDEL) src$(S)*.o 
	$(CDEL) framework$(S)*.o

$(APPNAME): $(SOURCES) include$(S)cpu_support.h
	$(CC) $(SOURCES) $(LDFLAGS) $(SDL_LDFLAGS) -o $@
	

