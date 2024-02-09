CC := $(shell !(command -v gcc >/dev/null 2>&1 && echo gcc) && command -v clang >/dev/null 2>&1 && echo clang)
CFLAGS ?= -Wall -Wextra -march=native
DEBUG_FLAGS ?= -Og -g -ggdb
RELEASE_FLAGS ?= -O3 -flto
DEBUGGER := $(shell command -v gdb)

all: release

release: clean release_compile exec

debug: clean compile exec

debug_compile: override CFLAGS += ${DEBUG_FLAGS}
debug_compile: compile

release_compile: override CFLAGS += ${RELEASE_FLAGS}
release_compile: compile

asm: compiler_check
	$(CC) -S -fverbose-asm $(CFLAGS) flat_hash_set.c

compile: compiler_check
compile:
	$(CC) $(CFLAGS) -c flat_hash_set.c -o flat_hash_set.o
	$(CC) $(CFLAGS) test.c flat_hash_set.o -o fhs_test

compiler_check:
ifeq ($(CC),)
	$(error Please specify a compiler)
endif

exec:
	./fhs_test

clean:
	$(RM) flat_hash_set.o flat_hash_set.s test.s fhs_test*
