# ALLang
> Another LISP Language.

### Instructions (num = 4)
1. "define"
2. "if"
3. "include"
4. "load"

### LOAD .Files VMS
1. add.vms "+"
2. sub.vms "-"
3. mul.vms "*"
4. div.vms "/"
5. lr.vms  "<"
6. gr.vms  ">"
7. eq.vms  "="
8. neq.vms "/="
9. ret.vms "return"

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
(load   include/vms/add.vms 
        include/vms/sub.vms 
        include/vms/lr.vms)

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

label +
    load -3
    load -3
    add
    store -4 -1
    pop
    ret

label -
    load -3
    load -3
    sub
    store -4 -1
    pop
    ret

label <
    load -3
    load -3
    jl _<_0
    jmp _<_1
label _<_0
    push 1
    jmp _<_end
label _<_1
    push 0
    jmp _<_end
label _<_end
    store -4 -1
    pop
    ret

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
```

### Output .File main.vme
```
0000 0000 000d 0000 006b 010f 0cff ffff
fd0c ffff fffd 020b ffff fffc ffff ffff
010e 0cff ffff fd0c ffff fffd 030b ffff
fffc ffff ffff 010e 0cff ffff fd0c ffff
fffd 0700 0000 4c06 0000 0056 0000 0000
0106 0000 0060 0000 0000 0006 0000 0060
0bff ffff fcff ffff ff01 0e06 0000 0070
0000 0000 0a0d 0000 0085 0bff ffff fdff
ffff ff01 0e0c ffff fffe 0600 0000 8f0c
ffff ffff 0000 0000 020d 0000 0038 0100
0000 0000 0a00 0000 ae06 0000 00c3 0000
0000 000c ffff fffe 0d00 0000 0c01 0600
0000 f30c ffff ffff 0000 0000 010d 0000
0022 010d 0000 0085 0cff ffff fe00 0000
0002 0d00 0000 2201 0d00 0000 850d 0000
000c 010b ffff fffc ffff ffff 0101 0e
```
