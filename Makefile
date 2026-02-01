CC=gcc
CFLAGS=-Iinclude

all: fft_demo filter_demo

fft_demo: src/fft.c examples/fft_demo.c
	$(CC) src/fft.c examples/fft_demo.c $(CFLAGS) -o build/fft_demo

filter_demo: src/filter.c examples/filter_demo.c
	$(CC) src/filter.c examples/filter_demo.c $(CFLAGS) -o build/filter_demo

clean:
	rm -f build/*
