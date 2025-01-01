CC=gcc
CFLAGS=-Wall -std=c99
FILES=all.c allkernel.c cvm/typeslib/list.c cvm/typeslib/hashtab.c  

.PHONY: default all install build run clean 
default: buildcvm build run

install:
	rm -rf cvm
	git clone -b v1.0.7 https://github.com/number571/cvm.git || true
buildcvm:
	make build -C ./cvm
	cp cvm/cvm ./_cvm
build: 
	$(CC) -o all $(CFLAGS) $(FILES)
	./all build main.all -o main.asm
run: 
	./_cvm build main.asm -o main.bcd
	./_cvm run main.bcd 5
clean:
	rm -f main.asm main.bcd cvm _cvm
