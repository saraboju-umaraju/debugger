CC := gcc
RM := /bin/rm -rf
ECHO := echo
CFLAGS += -ggdb3
CFLAGS += -Wall
CFLAGS += -Werror
TARGET_ARCH = 
TRACEE := tracee.c

SOURCES += do_debug.c
SOURCES += parent.c
SOURCES += child.c
SOURCES += list.c
SOURCES += docmd.c
OBJECTS := $(patsubst %.c, %.o, $(SOURCES))

cdb : $(OBJECTS) tracee
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)
clean :
	@$(RM) $(OBJECTS) cdb tracee tracee.o

tracee : tracee.o
	$(CC) $(CFLAGS) -o $@ $^
