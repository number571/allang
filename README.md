# ALLang
> Another LISP Language.

### Instructions (num = 3)
1. "define"
2. "if"
3. "include"

### LOAD .Files VMS
1. lib/init.vms (entry point to main function)
2. lib/add.vms  "+"
3. lib/sub.vms  "-"

### ALL interface functions
```c
// translate source file (input) into assembler listing file (output)
extern int all_compile(FILE *output, FILE *input);
```

### For compile and run need install CVM (version 1.0.3)
1. CVM: [github.com/number571/CVM](https://github.com/number571/CVM);

### Install CVM, compile .Files ALL, VMS and run .File VME
```
$ make install
$ make build
$ make run
```

### Input .File main.all
```scheme
(include 
	lib/init.vms
	lib/add.vms)

(define (isnull? x)
	(if (+ 0 x)
		(+ 0 0)
		(+ 0 1)))

(define (main)
	(if (isnull? 0)
		(+ 0 25)
		(+ 0 50)))
```

### Output .File main.vms
```asm
labl _start
	push main
	call
	hlt
labl +
	push -3
	load
	push -3
	load
	add
	push -1
	push -4
	stor
	pop
	jmp
labl isnull?
	push -2
	load
	push 0
	push -2
	load
	push +
	call
	pop
	push 0
	push _if_0
	jne
	push _else_0
	jmp
labl _if_0
	push 0
	push 0
	push +
	call
	pop
	push _end_0
	jmp
labl _else_0
	push 0
	push 1
	push +
	call
	pop
labl _end_0
	push -1
	push -4
	stor
	pop
	pop
	jmp
labl main
	push 0
	push isnull?
	call
	push 0
	push _if_1
	jne
	push _else_1
	jmp
labl _if_1
	push 0
	push 25
	push +
	call
	pop
	push _end_1
	jmp
labl _else_1
	push 0
	push 50
	push +
	call
	pop
labl _end_1
	push -1
	push -3
	stor
	pop
	jmp
```

### Output .File main.vme
```
0a00 0000 801d 1e0a ffff fffd 1c0a ffff
fffd 1c0c 0aff ffff ff0a ffff fffc 1b0b
f10a ffff fffe 1c0a 0000 0000 0aff ffff
fe1c 0a00 0000 071d 0b0a 0000 0000 0a00
0000 4aa2 0a00 0000 61f1 0a00 0000 000a
0000 0000 0a00 0000 071d 0b0a 0000 0072
f10a 0000 0000 0a00 0000 010a 0000 0007
1d0b 0aff ffff ff0a ffff fffc 1b0b 0bf1
0a00 0000 000a 0000 0021 1d0a 0000 0000
0a00 0000 9ca2 0a00 0000 b3f1 0a00 0000
000a 0000 0019 0a00 0000 071d 0b0a 0000
00c4 f10a 0000 0000 0a00 0000 320a 0000
0007 1d0b 0aff ffff ff0a ffff fffd 1b0b
f1
```
