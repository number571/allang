# ALLang
> Another LISP Language.

### Default procedures (num = 8)
1. +
2. -
3. *
4. /
5. <
6. >
7. =
8. /=

### Default instructions (num = 2)
1. define
2. if

### Tall interface functions
```c
// translate source file (input) into assembler listing file (output)
extern int8_t readtall_src(FILE *output, FILE *input)
```

### Translate, compile and run
```
$ make -C extclib/
$ make build
$ make trun
$ make vmrun
```

### Input .File main.all
```lisp
(define (main)
    (fact 10))

(define (fact x)
    (if (= x 0) 
        (+ 0 1)
        (* x (fact (- x 1)))))
```

### Output .File main.vms
```asm
label _start
    push 0
    call main
    pop
    hlt

; args: 0
label main
    push 10
    call fact
label main_end
    store $-3 $-1
    pop
    ret

; args: 1
label fact
    load $-2
    load $-1
    push 0
    call =
    pop
    push 1
    je _lbl_0
    jmp _lbl_1
label _lbl_0
    push 0
    push 1
    call +
    pop
    jmp fact_end
label _lbl_1
    load $-1
    load $-2
    push 1
    call -
    pop
    call fact
    call *
    pop
    jmp fact_end
label fact_end
    store $-4 $-1
    pop
    pop
    ret
...
```

### Output .File main.vme
```
0000 0000 000d 0000 000c 010f 0000 0000
0a0d 0000 0021 0bff ffff fdff ffff ff01
0e0c ffff fffe 0cff ffff ff00 0000 0000
0d00 0001 4901 0000 0000 0109 0000 0045
0600 0000 5a00 0000 0000 0000 0000 010d
0000 008b 0106 0000 007f 0cff ffff ff0c
ffff fffe 0000 0000 010d 0000 00a1 010d
0000 0021 0d00 0000 b701 0600 0000 7f0b
ffff fffc ffff ffff 0101 0e0c ffff fffd
0cff ffff fd02 0bff ffff fcff ffff ff01
0e0c ffff fffd 0cff ffff fd03 0bff ffff
fcff ffff ff01 0e0c ffff fffd 0cff ffff
fd04 0bff ffff fcff ffff ff01 0e0c ffff
fffd 0cff ffff fd05 0bff ffff fcff ffff
ff01 0e0c ffff fffd 0cff ffff fd07 0000
00f7 0600 0001 0100 0000 0001 0600 0001
0b00 0000 0000 0600 0001 0b0b ffff fffc
ffff ffff 010e 0cff ffff fd0c ffff fffd
0800 0001 2a06 0000 0134 0000 0000 0106
0000 013e 0000 0000 0006 0000 013e 0bff
ffff fcff ffff ff01 0e0c ffff fffd 0cff
ffff fd09 0000 015d 0600 0001 6700 0000
0001 0600 0001 7100 0000 0000 0600 0001
710b ffff fffc ffff ffff 010e 0cff ffff
fd0c ffff fffd 0a00 0001 9006 0000 019a
0000 0000 0106 0000 01a4 0000 0000 0006
0000 01a4 0bff ffff fcff ffff ff01 0e
```
