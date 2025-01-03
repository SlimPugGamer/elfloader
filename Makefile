TARGET = main
EE_BIN = $(TARGET).elf
EE_CFLAGS += -DNEWLIB_PORT_AWARE
EE_LIBS += -lpatches
EE_INCS += -Isrc/include -I$(PS2DEV)/gsKit/include -I$(PS2SDK)/ports/include -I$(PS2SDK)/ports/include/freetype2 -I$(PS2SDK)/ports/include/zlib
EE_OBJS += stage2/main.o
all: $(EE_BIN)

clean:
	rm -f $(EE_BIN) $(EE_OBJS)

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
