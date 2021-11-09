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
(include vms
	lib/vms/init.vms)

(include all
	lib/all/lr.all
	lib/all/add.all
	lib/all/sub.all
	lib/all/mul.all)

; f(x) = 1, if x < 1
; f(x) = x * f(x-1)
(define (fact x)
	(if (lr x 1) 
		(add 0 1)
		(mul x (fact (sub x 1)))))

; result: 120
(define (main)
	(fact 5))
```

### Output .File main.vms
```asm
labl _start
	push main
	call
	hlt
labl _add
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
labl _eq
    push -3
	load
    push -3
	load
    push _if_eq
	je
	push _else_eq
	jmp 
labl _if_eq
	push 1
	push _end_eq
	jmp 
labl _else_eq
	push 0
labl _end_eq
    push -1
    push -4
	stor
	pop
	jmp
labl not
	push -2
	load
	push -1
	load
	push 0
	push _eq
	call
	pop
	push 0
	push _if_0
	jne
	push _else_0
	jmp
labl _if_0
	push 0
	push 1
	push _add
	call
	pop
	push _end_0
	jmp
labl _else_0
	push 0
	push 0
	push _add
	call
	pop
labl _end_0
	push -1
	push -4
	stor
	pop
	pop
	jmp
labl _gr
    push -3
	load
    push -3
	load
    push _if_gr
	jg
	push _else_gr
	jmp 
labl _if_gr
	push 1
	push _end_gr
	jmp 
labl _else_gr
	push 0
labl _end_gr
    push -1
    push -4
	stor
	pop
	jmp
labl neq
	push -3
	load
	push -3
	load
	push -2
	load
	push -2
	load
	push _eq
	call
	pop
	push not
	call
	push -1
	push -6
	stor
	pop
	pop
	pop
	jmp
labl or
	push -3
	load
	push -3
	load
	push -2
	load
	push 0
	push neq
	call
	pop
	push 0
	push _if_1
	jne
	push _else_1
	jmp
labl _if_1
	push 0
	push 1
	push _add
	call
	pop
	push _end_1
	jmp
labl _else_1
	push -1
	load
	push 0
	push neq
	call
	pop
	push 0
	push _if_2
	jne
	push _else_2
	jmp
labl _if_2
	push 0
	push 1
	push _add
	call
	pop
	push _end_2
	jmp
labl _else_2
	push 0
	push 0
	push _add
	call
	pop
labl _end_2
labl _end_1
	push -1
	push -6
	stor
	pop
	pop
	pop
	jmp
labl ge
	push -3
	load
	push -3
	load
	push -2
	load
	push -2
	load
	push _eq
	call
	pop
	push -3
	load
	push -3
	load
	push _gr
	call
	pop
	push or
	call
	pop
	push -1
	push -6
	stor
	pop
	pop
	pop
	jmp
labl lr
	push -3
	load
	push -3
	load
	push -2
	load
	push -2
	load
	push ge
	call
	pop
	push not
	call
	push -1
	push -6
	stor
	pop
	pop
	pop
	jmp
labl add
	push -3
	load
	push -3
	load
	push -2
	load
	push -2
	load
	push _add
	call
	pop
	push -1
	push -6
	stor
	pop
	pop
	pop
	jmp
labl _sub
	push -3
	load
	push -3
	load
	sub
	push -1
	push -4
	stor
	pop
	jmp
labl sub
	push -3
	load
	push -3
	load
	push -2
	load
	push -2
	load
	push _sub
	call
	pop
	push -1
	push -6
	stor
	pop
	pop
	pop
	jmp
labl _mul
	push -3
	load
	push -3
	load
	mul
	push -1
	push -4
	stor
	pop
	jmp
labl mul
	push -3
	load
	push -3
	load
	push -2
	load
	push -2
	load
	push _mul
	call
	pop
	push -1
	push -6
	stor
	pop
	pop
	pop
	jmp
labl fact
	push -2
	load
	push -1
	load
	push 1
	push lr
	call
	pop
	push 0
	push _if_3
	jne
	push _else_3
	jmp
labl _if_3
	push 0
	push 1
	push add
	call
	pop
	push _end_3
	jmp
labl _else_3
	push -1
	load
	push -2
	load
	push 1
	push sub
	call
	pop
	push fact
	call
	push mul
	call
	pop
labl _end_3
	push -1
	push -4
	stor
	pop
	pop
	jmp
labl main
	push 5
	push fact
	call
	push -1
	push -3
	stor
	pop
	jmp
```

### Output .File main.vme
```
0a00 0003 6b1d 1e0a ffff fffd 1c0a ffff
fffd 1c0c 0aff ffff ff0a ffff fffc 1b0b
f10a ffff fffd 1c0a ffff fffd 1c0a 0000
0039 1a0a 0000 0044 f10a 0000 0001 0a00
0000 49f1 0a00 0000 000a ffff ffff 0aff
ffff fc1b 0bf1 0aff ffff fe1c 0aff ffff
ff1c 0a00 0000 000a 0000 0021 1d0b 0a00
0000 000a 0000 007f a20a 0000 0096 f10a
0000 0000 0a00 0000 010a 0000 0007 1d0b
0a00 0000 a7f1 0a00 0000 000a 0000 0000
0a00 0000 071d 0b0a ffff ffff 0aff ffff
fc1b 0b0b f10a ffff fffd 1c0a ffff fffd
1c0a 0000 00cd 0f0a 0000 00d8 f10a 0000
0001 0a00 0000 ddf1 0a00 0000 000a ffff
ffff 0aff ffff fc1b 0bf1 0aff ffff fd1c
0aff ffff fd1c 0aff ffff fe1c 0aff ffff
fe1c 0a00 0000 211d 0b0a 0000 0056 1d0a
ffff ffff 0aff ffff fa1b 0b0b 0bf1 0aff
ffff fd1c 0aff ffff fd1c 0aff ffff fe1c
0a00 0000 000a 0000 00ea 1d0b 0a00 0000
000a 0000 014d a20a 0000 0164 f10a 0000
0000 0a00 0000 010a 0000 0007 1d0b 0a00
0001 aff1 0aff ffff ff1c 0a00 0000 000a
0000 00ea 1d0b 0a00 0000 000a 0000 0187
a20a 0000 019e f10a 0000 0000 0a00 0000
010a 0000 0007 1d0b 0a00 0001 aff1 0a00
0000 000a 0000 0000 0a00 0000 071d 0b0a
ffff ffff 0aff ffff fa1b 0b0b 0bf1 0aff
ffff fd1c 0aff ffff fd1c 0aff ffff fe1c
0aff ffff fe1c 0a00 0000 211d 0b0a ffff
fffd 1c0a ffff fffd 1c0a 0000 00b5 1d0b
0a00 0001 1e1d 0b0a ffff ffff 0aff ffff
fa1b 0b0b 0bf1 0aff ffff fd1c 0aff ffff
fd1c 0aff ffff fe1c 0aff ffff fe1c 0a00
0001 be1d 0b0a 0000 0056 1d0a ffff ffff
0aff ffff fa1b 0b0b 0bf1 0aff ffff fd1c
0aff ffff fd1c 0aff ffff fe1c 0aff ffff
fe1c 0a00 0000 071d 0b0a ffff ffff 0aff
ffff fa1b 0b0b 0bf1 0aff ffff fd1c 0aff
ffff fd1c 0d0a ffff ffff 0aff ffff fc1b
0bf1 0aff ffff fd1c 0aff ffff fd1c 0aff
ffff fe1c 0aff ffff fe1c 0a00 0002 681d
0b0a ffff ffff 0aff ffff fa1b 0b0b 0bf1
0aff ffff fd1c 0aff ffff fd1c c00a ffff
ffff 0aff ffff fc1b 0bf1 0aff ffff fd1c
0aff ffff fd1c 0aff ffff fe1c 0aff ffff
fe1c 0a00 0002 b01d 0b0a ffff ffff 0aff
ffff fa1b 0b0b 0bf1 0aff ffff fe1c 0aff
ffff ff1c 0a00 0000 010a 0000 0206 1d0b
0a00 0000 000a 0000 0321 a20a 0000 0338
f10a 0000 0000 0a00 0000 010a 0000 023a
1d0b 0a00 0003 5df1 0aff ffff ff1c 0aff
ffff fe1c 0a00 0000 010a 0000 0282 1d0b
0a00 0002 f81d 0a00 0002 ca1d 0b0a ffff
ffff 0aff ffff fc1b 0b0b f10a 0000 0005
0a00 0002 f81d 0aff ffff ff0a ffff fffd
1b0b f1
```
