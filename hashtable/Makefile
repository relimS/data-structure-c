CC := $(shell !(command -v clang >/dev/null 2>&1 && echo clang) && command -v >/dev/null 2>&1 && echo gcc)
CFLAGS ?= -Wall -Wextra
DEBUG_FLAGS ?= -Og -g -ggdb
RELEASE_FLAGS ?= -O2
DEBUGGER := $(shell command -v gdb)

all: release

debug: CFLAGS += ${DEBUG_FLAGS}
debug: clean main debugger_exec

release: CFLAGS += ${RELEASE_FLAGS}
release: clean main exec
main:
	$(CC) $(CFLAGS) -c hashtable.c -o hashtable.o
	$(CC) $(CFLAGS) test.c hashtable.o -o ht_test

exec:
	./ht_test

debugger_exec:
ifeq ($(strip $(DEBUGGER)),)
	./ht_test
else
	@echo run | $(DEBUGGER) ./ht_test* && echo
endif

clean:
	$(RM) hashtable.o ht_test*