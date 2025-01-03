TARGET = main
EE_BIN = $(TARGET).elf
EE_CFLAGS += -DNEWLIB_PORT_AWARE
EE_LIBS += -lpatches -lgskit
EE_OBJS += main.o
all: $(EE_BIN)

clean:
	rm -f $(EE_BIN) $(EE_OBJS)

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal