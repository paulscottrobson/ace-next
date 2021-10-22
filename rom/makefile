ifeq ($(OS),Windows_NT)
include ..\documents\common.make
else
include ../documents/common.make
endif


.PHONY: all 

all: 
#	zasm -u -b rom.listing -o assembled.rom.bin -l assembled.rom.lst
	cmp assembled.rom.bin rom.bin
	ruby export.rb
	ruby scancodes.rb
	ruby keyboard.rb
