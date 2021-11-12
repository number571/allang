# ALLang
> Another LISP Language is a purely functional programming language. Version 1.0.2.

#### Instructions (num = 3)
1. "define"
2. "if"
3. "include"

#### ALL interface function
```c
// translate source file (input) into assembly listing file (output)
extern int all_compile(FILE *output, FILE *input);
```

#### For compile and run need install CVM (version 1.0.5)
1. CVM: [github.com/number571/CVM](https://github.com/number571/CVM/tree/v1.0.5);

#### ALL used only main CVM instructions (num = 11)

Bytecode | Stack | Args | Instruction
:---: | :---: | :---: | :---: |
0x0A | 0 | 1 | push
0x0B | 1 | 0 | pop
0x0C | 1 | 0 | inc
0x0D | 1 | 0 | dec
0x0E | 3 | 0 | jg
0x0F | 3 | 0 | je
0x1A | 1 | 0 | jmp
0x1B | 2 | 0 | stor
0x1C | 1 | 0 | load
0x1D | 1 | 0 | call
0x1E | 0 | 0 | hlt

#### Install CVM, compile .Files ALL, VMS and run .File VME
```
$ make install
$ make build
$ make run
```

#### Input .File main.all
```scheme
(include vms
	lib/vms/init.vms)

(include all
	lib/all/lr.all
	lib/all/ret.all
	lib/all/dec.all
	lib/all/mul.all)

; result: 120
(define (main)
	(fact 5))

; f(x) = 1, if x < 1
; f(x) = x * f(x-1)
(define (fact x)
	(if (lr x 1) 
		(ret 1)
		(mul x (fact (dec x)))))
```

#### ALL based on low-level functions from `lib/vms` (num = 4)
Function | Args | Result
:---: | :---: | :---: |
_inc | x | (x + 1)
_dec | x | (x - 1)
_eq | x, y | (x = y)
_gr | x, y | (x > y)

#### ALL high-level functions from `lib/all` (num = 22)

Function | Args | Result
:---: | :---: | :---: |
add | x, y | (x + y)
mul | x, y | (x * y)
sub | x, y | (x - y)
div | x, y | (x / y)
mod | x, y | (x % y)
and | x, y | x && y
or | x, y | x &vert;&vert; y
xor | x, y | x ^ y
not | x | ~x
shl | x, y | x << y
shr | x, y | x >> y
ret | x | x
inc | x | x + 1
dec | x | x - 1
abs | x | &vert;x&vert;
eq | x, y | x = y
lr | x, y | x < y
gr | x, y | x > y
le | x, y | x >= y
ge | x, y | x <= y
neq | x, y | x ~= y
neg | x | -x
