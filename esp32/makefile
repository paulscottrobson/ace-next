ifeq ($(OS),Windows_NT)
include ..\documents\common.make
else
include ../documents/common.make
endif

EMUROOT = ..$(S)emulator$(S)
EMUSRC = $(EMUROOT)src$(S)

.PHONY: all serial update

all: update
	pio run -t uploadfs	

serial: update monitor

monitor:
	pio device monitor -b 115200
	
update:
	make -C ..$(S)emulator
	$(CCOPY) $(EMUSRC)hardware.cpp src	
	$(CCOPY) $(EMUSRC)hardware_esp.cpp src	
	$(CCOPY) $(EMUSRC)hwtape.cpp src	
	$(CCOPY) $(EMUSRC)sys_processor.cpp src	
	$(CCOPY) $(EMUROOT)include/*.* include
	$(CCOPY) $(EMUROOT)roms/*.* include
	$(CCOPY) $(EMUROOT)cpu/*.* include
	$(CCOPY) $(EMUROOT)framework/gfxkeys.h include
	$(CCOPY) $(EMUROOT)storage/*.* data
	pio lib install 6143 	
	pio run -t upload
