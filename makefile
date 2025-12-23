CC=gcc
CFLAGS=-O2 -Wall  
TARGET=scaramouche

all: $(TARGET)

$(TARGET): scaramouche.o onion.o tor.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f *.o $(TARGET)

.PHONY: all clean