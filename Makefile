CC := gcc
CLANG := clang
RM := /bin/rm -rf
PRINTF := printf
ECHO := echo
CFLAGS += -DCHILD_PROCESS="\"cdb\""
#CFLAGS += -DCHILD_PROCESS="\"do_me\""
CFLAGS += -gdwarf-2
CFLAGS += -ggdb3 -rdynamic
CFLAGS += -Wfatal-errors
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
SOURCES += do_elf.c
SOURCES += do_elf_frame.c
SOURCES += do_elf_common.c
SOURCES += do_elf_lines.c
SOURCES += do_elf_attr.c
SOURCES += do_signal.c
ifeq ($(DEBUG), 1)
  ALLOC_FLAGS += -Wl,--wrap=malloc -Wl,--wrap=free -DALLOC_DEBUG
endif
CFLAGS += $(ALLOC_FLAGS)
OBJECTS := $(patsubst %.c, %.o, $(SOURCES))

cdb : $(OBJECTS) do_me 
	@$(CC) $(CFLAGS) -o $@ $(OBJECTS)
	@$(PRINTF) 'LINK %-10s\n' "$@"
	@./cdb
clean :
	@$(RM) $(OBJECTS) cdb do_me do_me.o dwarf dwarf.o for-dwarf for-dwarf.o do_me_helper.o

for-dwarf.o : for-dwarf.c
	@$(CLANG) -gdwarf-2 -c -o $@ $^
do_me.o : do_me.c
	@$(CLANG) -gdwarf-2 -c -o $@ $^
do_me_helper.o : do_me_helper.c
	@$(CLANG) -gdwarf-2 -c -o $@ $^

%.o : %.c 
	@$(CC) $(CFLAGS) -c -o $@ $^
	@$(PRINTF) 'COMPILE %-10s\n' "$<"

do_me : do_me.o do_me_helper.o
	@$(CLANG) $(CFLAGS) -o $@ $^
	@$(PRINTF) 'LINK %-10s\n' "$@"

dwarf : dwarf.o for-dwarf
	@$(CC) $< -o $@
	@$(PRINTF) 'LINK %-10s\n' "$@"
	@./dwarf

for-dwarf: for-dwarf.o
	@$(CLANG) $< -o $@
	@$(PRINTF) 'LINK %-10s\n' "$@"
