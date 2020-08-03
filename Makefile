CC=gcc
CFLAGS=-Wall -std=c99

FILES=ctall.c readtall.c extclib/extclib.o

.PHONY: default build vmrun trun clean
default: build trun vmrun 

build: $(FILES)
	$(CC) $(CFLAGS) $(FILES) -o ctall
trun: ctall
	./ctall build main.all
vmrun: cvm 
	./cvm build main.vms -o main.vme
	./cvm run main.vme
clean:
	rm -f ctall main.vme
