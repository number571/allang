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

#### ALL based on low-level functions from `lib/vms` (num = 4)
1. inc(x):   (x + 1)
2. dec(x):   (x - 1)
3. eq(x, y): (x = y)
4. gr(x, y): (x > y)

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

#### ALL used only main CVM instructions (num = 11)
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

#### ALL high-level functions from `lib/all` (num = 22)
1. add(x, y):  (x + y)
2. mul(x, y):  (x * y)
3. sub(x, y):  (x - y)
4. div(x, y):  (x / y)
5. mod(x, y):  (x % y)
6. and(x, y):  (x && y)
7. or(x, y):   (x || y)
8. xor(x, y):  (x ^ y)
9. not(x):     (~x)
10. shl(x, y): (x << y)
11. shr(x, y): (x >> y)
12. ret(x):    (x)
13. inc(x):    (x + 1)
14. dec(x):    (x - 1)
15. abs(x):    (x < 0 ? -x : x)
16. eq(x, y):  (x = y)
17. lr(x, y):  (x < y)
18. gr(x, y):  (x > y)
19. le(x, y):  (x <= y)
20. ge(x, y):  (x >= y)
21. neq(x, y): (x != y)
22. neg(x):    (-x)
