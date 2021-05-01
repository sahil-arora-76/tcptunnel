
COMPILER=gcc

FLAGS=-Wall \
	-lpthread

RED=\033[31m

BLUE=\003[34m

FILE=src/tcptunnel.c

build/tunnel.o: $(FILE)
	$(COMPILER) -o $@ $< $(FLAGS)
clean: 
	rm -rf build