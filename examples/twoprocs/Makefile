#CFLAGS = -DDEBUG
CFLAGS =

beowulfpc: twoprocs.c
	mkdir build
	gcc $(CFLAGS) -o build/twoprocs  twoprocs.c -lrt -pthread

clean:
	rm -f build/twoprocs
