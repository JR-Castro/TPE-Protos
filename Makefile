CFLAGS= -g --std=c11 -pedantic -pedantic-errors -Wall -Wextra -Wno-unused-parameter -Wno-newline-eof -Wno-implicit-fallthrough -D_POSIX_C_SOURCE=200112L -fsanitize=address

LDFLAGS= -pthread

INCLUDE=-I src/include

SOURCES=src/buffer.c src/main.c src/netutils.c src/parser.c src/parser_utils.c src/selector.c src/stm.c src/pop3.c
OBJECTS=$(SOURCES:.c=.o)

TESTS=tests/buffer_test.c tests/netutils_test.c tests/parser_test.c tests/parser_utils_test.c tests/selector_test.c tests/stm_test.c
TEST_OUTPUTS=$(TESTS:.c=.out)

OUTPUT=server

all: $(SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) $(INCLUDE) -o $(OUTPUT) $(SOURCES)

test: $(TEST_OUTPUTS)

tests/%.out: tests/%.c
	$(CC) $(CFLAGS) -lcheck $(INCLUDE) $< -o $@

check: clean
	pvs-studio-analyzer trace -- make
	pvs-studio-analyzer analyze
	plog-converter -a '64:1,2,3;GA:1,2,3;OP:1,2,3' -t tasklist -o ./report.tasks ./PVS-Studio.log

	rm PVS-Studio.log
	mv strace_out check

clean:
	rm -rf src/*.o *.o $(OUTPUT)

.PHONY: all clean