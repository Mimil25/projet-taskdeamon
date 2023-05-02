C = clang -Wall -Wextra -pedantic -g -gdwarf-4 -O0 -std=c99

init:
	mkdir bin
	mkdir lib
	mkdir test/bin

all: time when libmessage

time:
	$C src/time.c -o bin/time

when:
	$C src/when.c -o bin/when

libmessage:
	$C -c -fpic src/message.c -o lib/message.o
	$C -shared lib/message.o -o lib/libmessage.so

test: test_libmessage

test_libmessage: libmessage
	$C lib/message.o test/src/test_message_a.c -o test/bin/test_message_a
	$C lib/message.o test/src/test_message_b.c -o test/bin/test_message_b

clean:
	rm -rf bin/
	rm -rf lib/
	rm -rf test/bin/
