# ALLang
> Another LISP Language. Version 1.0.1.

### Instructions (num = 3)
1. "define"
2. "if"
3. "include"

### ALL interface function
```c
// translate source file (input) into assembly listing file (output)
extern int all_compile(FILE *output, FILE *input);
```

### For compile and run need install CVM (version 1.0.4)
1. CVM: [github.com/number571/CVM](https://github.com/number571/CVM);

### ALL used only main CVM instructions (num = 11)
1.  [0x0A] "push"
2.  [0x0B] "pop"
3.  [0x0C] "inc"
4.  [0x0D] "dec"
5.  [0x0E] "jg"
6.  [0x0F] "je"
7.  [0x1A] "jmp"
8.  [0x1B] "stor"
9.  [0x1C] "load"
10. [0x1D] "call"
11. [0x1E] "hlt"

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
