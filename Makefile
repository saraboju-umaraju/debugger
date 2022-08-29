CC := gcc
RM := /bin/rm -rf
PRINTF := printf
ECHO := echo
CFLAGS += -ggdb3 -rdynamic
CFLAGS += -Wall
CFLAGS += -Werror
TARGET_ARCH = 
TRACEE := do_me.c

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

cdb : $(OBJECTS) do_me 
	@$(CC) $(CFLAGS) -o $@ $(OBJECTS)
	@$(PRINTF) 'LINK %-10s\n' "$@"
	./cdb
clean :
	@$(RM) $(OBJECTS) cdb do_me do_me.o

%.o : %.c 
	@$(CC) $(CFLAGS) -c -o $@ $^
	@$(PRINTF) 'COMPILE %-10s\n' "$<"

do_me : do_me.o
	@$(CC) $(CFLAGS) -o $@ $^
	@$(PRINTF) 'LINK %-10s\n' "$@"
