CC=gcc
CFLAGS=-Wall -std=c99

FILES=all.c allkernel.c CVM/extclib/type/list.c  CVM/extclib/type/hashtab.c  

.PHONY: default all install build run clean 
default: build run
install: 
	git clone -b v1.0.4 https://github.com/number571/CVM.git || true
	make install -C ./CVM
	make build -C ./CVM
	cp CVM/cvm .
build: 
	$(CC) -o all $(CFLAGS) $(FILES)
	./all build main.all -o main.vms
run: 
	./cvm build main.vms -o main.vme
	./cvm run main.vme 0
clean:
	rm -f main.vms main.vme
