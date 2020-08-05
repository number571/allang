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
label _start
    push 0
    call main
    pop
    hlt

; argc: 0
label main
    jmp _main_begin

label _main_begin
    push 10
    call fib
label _main_end
    store $-3 $-1
    pop
    ret

; argc: 1
label fib
    load $-2
    jmp _fib_begin

label _fib_begin
    load $-1
    push 2
    call <
    pop
    push 0
    jne _lbl_0
    jmp _lbl_1
label _lbl_0
    push 0
    load $-2
    call +
    pop
    jmp _fib_end
label _lbl_1
    load $-1
    push 1
    call -
    pop
    call fib
    load $-2
    push 2
    call -
    pop
    call fib
    call +
    pop
    jmp _fib_end
label _fib_end
    store $-4 $-1
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
0cff ffff ff00 0000 0002 0d00 0000 fd01
0000 0000 000a 0000 004f 0600 0000 6400
0000 0000 0cff ffff fe0d 0000 00a5 0106
0000 0099 0cff ffff ff00 0000 0001 0d00
0000 bb01 0d00 0000 260c ffff fffe 0000
0000 020d 0000 00bb 010d 0000 0026 0d00
0000 a501 0600 0000 990b ffff fffc ffff
ffff 0101 0e0c ffff fffd 0cff ffff fd02
0bff ffff fcff ffff ff01 0e0c ffff fffd
0cff ffff fd03 0bff ffff fcff ffff ff01
0e0c ffff fffd 0cff ffff fd04 0bff ffff
fcff ffff ff01 0e0c ffff fffd 0cff ffff
fd05 0bff ffff fcff ffff ff01 0e0c ffff
fffd 0cff ffff fd07 0000 0111 0600 0001
1b00 0000 0001 0600 0001 2500 0000 0000
0600 0001 250b ffff fffc ffff ffff 010e
0cff ffff fd0c ffff fffd 0800 0001 4406
0000 014e 0000 0000 0106 0000 0158 0000
0000 0006 0000 0158 0bff ffff fcff ffff
ff01 0e0c ffff fffd 0cff ffff fd09 0000
0177 0600 0001 8100 0000 0001 0600 0001
8b00 0000 0000 0600 0001 8b0b ffff fffc
ffff ffff 010e 0cff ffff fd0c ffff fffd
0a00 0001 aa06 0000 01b4 0000 0000 0106
0000 01be 0000 0000 0006 0000 01be 0bff
ffff fcff ffff ff01 0e
```
