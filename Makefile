CC := gcc
RM := /bin/rm -rf
ECHO := echo
CFLAGS += -ggdb3 -rdynamic
CFLAGS += -Wall
CFLAGS += -Werror
TARGET_ARCH = 
TRACEE := tracee.c

SOURCES += do_debug.c
SOURCES += do_parent.c
SOURCES += do_child.c
SOURCES += do_command.c
SOURCES += do_continue.c
SOURCES += do_break.c
SOURCES += do_info.c
SOURCES += do_misc.c
SOURCES += do_register.c
SOURCES += do_quit.c
SOURCES += do_mem.c
OBJECTS := $(patsubst %.c, %.o, $(SOURCES))

cdb : $(OBJECTS) tracee 
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)
	./cdb
clean :
	@$(RM) $(OBJECTS) cdb tracee tracee.o

tracee : tracee.o
	$(CC) $(CFLAGS) -o $@ $^
