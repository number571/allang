CC=gcc
CFLAGS=-Wall -std=c99

FILES=ctall.c readtall.c extclib/extclib.o

.PHONY: default install build vmrun trun clean
default: build trun vmrun 

install:
	git clone https://github.com/number571/CVM.git || true
	make -C CVM/extclib/
	make build -C CVM/
	cp CVM/cvm .
	git clone https://github.com/number571/extclib.git || true
	make -C extclib/
build: $(FILES)
	$(CC) -o ctall $(CFLAGS) $(FILES) -lcrypto
trun: ctall
	./ctall build main.all -o main.vms
vrun: cvm 
	./cvm build main.vms -o main.vme
	./cvm run main.vme
clean:
	rm -rf extclib/ CVM/
	rm -f cvm ctall main.vms main.vme
