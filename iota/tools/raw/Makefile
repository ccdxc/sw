all: txrx

SRCS := common.c txrx.c

CFLAGS := -g -std=c99
LIBS := -lpcap

txrx: $(SRCS)
	mkdir -p bin
	gcc $(CFLAGS) $(SRCS) $(LIBS) -o bin/txrx

clean:
	rm -f bin/txrx
