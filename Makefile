C = clang -Wall -Wextra -pedantic -g -gdwarf-4 -O0 -std=c99

all: time when libmessage

time:
	$C src/time.c -o bin/time

when:
	$C src/when.c -o bin/when

libmessage:
	$C -shared src/message.c -o lib/libmessage.so

clean:
	rm -f bin/*
	rm -f lib/*
