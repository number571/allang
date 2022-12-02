CC=gcc
CFLAGS=-Wall -std=c99
FILES=all.c allkernel.c cvm/extclib/type/list.c cvm/extclib/type/hashtab.c  

.PHONY: default all install build run clean 

default: build run

all: install build run clean 

install: 
	git clone -b v1.0.5 https://github.com/number571/cvm.git || true
	make install -C ./cvm
	make build -C ./cvm
	cp cvm/cvm ./_cvm

build: 
	$(CC) -o all $(CFLAGS) $(FILES)
	./all build main.all -o main.vms

run: 
	./_cvm build main.vms -o main.vme
	./_cvm run main.vme 5

clean:
	rm -f main.vms main.vme
