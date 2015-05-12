CC=gcc

LIBS=-luv

all: echo-server echo-client echo-middle

echo-server: echo-server.c
	$(CC) -g -o echo-server echo-server.c $(LIBS)

echo-client: echo-client.c
	$(CC) -g -o echo-client echo-client.c $(LIBS)

echo-middle: echo-middle.c
	$(CC) -g -o echo-middle echo-middle.c $(LIBS)

clean:
	rm -f echo-server
	rm -f echo-client
	rm -f echo-middle
