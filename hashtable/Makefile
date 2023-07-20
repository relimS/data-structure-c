CC := $(shell !(command -v gcc >/dev/null 2>&1 && echo gcc) && command -v clang >/dev/null 2>&1 && echo clang)
CFLAGS ?= -Wall -Wextra -fstrict-aliasing
DEBUG_FLAGS ?= -Og -g -ggdb
RELEASE_FLAGS ?= -O3 -flto
DEBUGGER := $(shell command -v gdb)

all: release

debug: override CFLAGS += ${DEBUG_FLAGS}
debug: clean main exec

release: override CFLAGS += ${RELEASE_FLAGS}
release: clean main exec

main:
ifeq ($(CC),)
	@echo "Error: Cannot find gcc or clang."
	@echo "Suggestion: make CC=<compiler path>"
else
	$(CC) $(CFLAGS) -c hashtable.c -o hashtable.o
	$(CC) $(CFLAGS) test.c hashtable.o -o ht_test
endif

exec:
	./ht_test

debugger_exec:
ifeq ($(DEBUGGER),)
	./ht_test
else
	@echo run | $(DEBUGGER) ./ht_test* && echo
endif

clean:
	$(RM) hashtable.o ht_test*