# ALLang
> Another LISP Language.

### Default procedures (num = 8)
1. "+"
2. "-"
3. "*"
4. "/"
5. "<"
6. ">"
7. "="
8. "/="

### Instructions (num = 2)
1. "define"
2. "if"

### Tall interface functions
```c
// translate source file (input) into assembler listing file (output)
extern int8_t readtall_src(FILE *output, FILE *input)
```

### For compile and run need install VM
1. CVM: [github.com/number571/VirtualMachine](https://github.com/number571/VirtualMachine);

### Translate, compile and run
```
$ make -C extclib/
$ make build
$ make trun
$ make vmrun
```

### Input .File main.all
```scheme
(define (main)
    (fib 10))

; f(x) = x, if x < 2
; f(x) = f(x-1) + f(x-2)
(define (fib x)
    (if (< x 2)
        (+ 0 x)
        (+ 
            (fib (- x 1)) 
            (fib (- x 2)))))
```

### Output .File main.vms
```asm
...
; argc: 0
label main
    jmp _main_begin

label _main_begin
    push 10
    call fib
label _main_end
    store -3 -1
    pop
    ret

; argc: 1
label fib
    load -2
    jmp _fib_begin

label _fib_begin
    load -1
    push 2
    call <
    pop
    push 0
    jne _fib_0
    jmp _fib_1
label _fib_0
    push 0
    load -2
    call +
    pop
    jmp _fib_end
label _fib_1
    load -1
    push 1
    call -
    pop
    call fib
    load -2
    push 2
    call -
    pop
    call fib
    call +
    pop
label _fib_end
    store -4 -1
    pop
    pop
    ret
...
```

### Output .File main.vme
```
0000 0000 000d 0000 000c 010f 0600 0000
1100 0000 000a 0d00 0000 260b ffff fffd
ffff ffff 010e 0cff ffff fe06 0000 0030
0cff ffff ff00 0000 0002 0d00 0001 0801
0000 0000 000a 0000 004f 0600 0000 6400
0000 0000 0cff ffff fe0d 0000 00b0 0106
0000 0094 0cff ffff ff00 0000 0001 0d00
0000 c601 0d00 0000 260c ffff fffe 0000
0000 020d 0000 00c6 010d 0000 0026 0d00
0000 b001 0bff ffff fcff ffff ff01 010e
0cff ffff fe0b ffff fffd ffff ffff 010e
0cff ffff fd0c ffff fffd 020b ffff fffc
ffff ffff 010e 0cff ffff fd0c ffff fffd
030b ffff fffc ffff ffff 010e 0cff ffff
fd0c ffff fffd 040b ffff fffc ffff ffff
010e 0cff ffff fd0c ffff fffd 050b ffff
fffc ffff ffff 010e 0cff ffff fd0c ffff
fffd 0700 0001 1c06 0000 0126 0000 0000
0106 0000 0130 0000 0000 0006 0000 0130
0bff ffff fcff ffff ff01 0e0c ffff fffd
0cff ffff fd08 0000 014f 0600 0001 5900
0000 0001 0600 0001 6300 0000 0000 0600
0001 630b ffff fffc ffff ffff 010e 0cff
ffff fd0c ffff fffd 0900 0001 8206 0000
018c 0000 0000 0106 0000 0196 0000 0000
0006 0000 0196 0bff ffff fcff ffff ff01
0e0c ffff fffd 0cff ffff fd0a 0000 01b5
0600 0001 bf00 0000 0001 0600 0001 c900
0000 0000 0600 0001 c90b ffff fffc ffff
ffff 010e 
```
