pwd = $(shell pwd)

objects = main.o jsmn.o

test: main.c libsunrise-sunset.so
	gcc -L$(pwd) -Wall -Werror -o test main.c -lsunrise-sunset
libsunrise-sunset.so: sunrise_sunset.o jsmn.o
	gcc -shared -Wall -o libsunrise-sunset.so sunrise_sunset.o jsmn.o
sunrise_sunset.o: sunrise_sunset.c jsmn.h
	gcc -c -Wall -Werror -fpic sunrise_sunset.c

.PHONY: install
install:
	cp libsunrise-sunset.so /usr/lib/
	chmod 0755 /usr/lib/libsunrise-sunset.so
	ldconfig
.PHONY: uninstall
uninstall:
	rm /usr/lib/libsunrise-sunset.so
	ldconfig
.PHONY: clean
clean:
	rm -f test libsunrise-sunset.so sunrise_sunset.o
