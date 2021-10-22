ifeq ($(OS),Windows_NT)
include ..\documents\common.make
else
include ../documents/common.make
endif

all: test.asm

split:
	ruby split.rb
	
test.asm: 
#	make -C ..$(S)emulator
	ruby maketest.rb
	zasm -u test.asm


