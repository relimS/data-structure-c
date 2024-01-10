CC := $(shell !(command -v gcc >/dev/null 2>&1 && echo gcc) && command -v clang >/dev/null 2>&1 && echo clang)
CFLAGS ?= -Wall -Wextra -march=native
DEBUG_FLAGS ?= -Og -g -ggdb
RELEASE_FLAGS ?= -O3 -flto
DEBUGGER := $(shell command -v gdb)

all: release

debug: override CFLAGS += ${DEBUG_FLAGS}
debug: clean compile exec

release: override CFLAGS += ${RELEASE_FLAGS}
release: clean compile exec

compile:
ifeq ($(CC),)
	@echo "Error: Cannot find gcc or clang."
	@echo "Suggestion: make CC=<compiler path>"
else
	$(CC) $(CFLAGS) -c flat_hash_set.c -o flat_hash_set.o
	$(CC) $(CFLAGS) test.c flat_hash_set.o -o fhs_test
endif

exec:
	./fhs_test

clean:
	$(RM) flat_hash_set.o fhs_test*
