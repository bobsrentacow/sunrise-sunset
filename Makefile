pwd = $(shell pwd)

objects = main.o jsmn.o

test: main.c libsunrise-sunset.so
	gcc -L$(pwd) -Wall -Werror -o test main.c -lm -lsunrise-sunset
libsunrise-sunset.so: sunrise_sunset.o jsmn.o
	gcc -shared -Wall -o libsunrise-sunset.so sunrise_sunset.o jsmn.o
sunrise_sunset.o: sunrise_sunset.c jsmn.h
	gcc -c -Wall -Werror -fpic sunrise_sunset.c
deb: libsunrise-sunset.so
	mkdir -p libsunrise-sunset/usr/local/lib
	cp libsunrise-sunset.so libsunrise-sunset/usr/local/lib
	cp -r DEBIAN libsunrise-sunset/
	dpkg-deb --build libsunrise-sunset
.PHONY: install
install:
	cp libsunrise-sunset.so /usr/local/lib/
	chmod 0755 /usr/local/lib/libsunrise-sunset.so
	ldconfig
.PHONY: uninstall
uninstall:
	rm /usr/local/lib/libsunrise-sunset.so
	ldconfig
.PHONY: clean
clean:
	rm -f test libsunrise-sunset.deb libsunrise-sunset.so sunrise_sunset.o
	rm -rf libsunrise-sunset/
