#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "extclib/hashtab.h"

#define BUFF_SIZE 256
#define INSTRC_NUM 2

enum {
    DEFINE_INSTRC,
    IF_INSTRC,
};

static const char *instructions[INSTRC_NUM] = {
    [DEFINE_INSTRC]    = "define",
    [IF_INSTRC]        = "if",
};

static size_t counter = 0;

static void _init_entry(FILE *output);
static void _init_labels(FILE *output);
static int8_t _parse_code(FILE *output, FILE *input, size_t argc, HashTab *hashtab, char *proc);

static int8_t _define_instrc(
    FILE *output, 
    FILE *input, 
    HashTab *nhashtab, 
    char *buffer, 
    char *operation
);
static int8_t _if_instrc(
    FILE *output, 
    FILE *input, 
    HashTab *nhashtab, 
    HashTab *hashtab, 
    char *buffer, 
    char *operation, 
    char *proc, 
    size_t argc
);
static int8_t _proc_instrc(
    FILE *output, 
    FILE *input, 
    HashTab *nhashtab, 
    HashTab *hashtab, 
    char *buffer, 
    char *operation, 
    char *proc, 
    size_t argc
);

static char *_read_proc(FILE *input, char *buffer, size_t size);
static int8_t _read_defop(char *str);

extern int8_t readtall_src(FILE *output, FILE *input) {
    HashTab *nhashtab = new_hashtab(25, STRING_TYPE, DECIMAL_TYPE);
    _init_entry(output);
    int res = _parse_code(output, input, 0, nhashtab, "");
    free_hashtab(nhashtab);
    _init_labels(output);
    return res;
}

static void _init_entry(FILE *output) {
    fprintf(output, "label _start\n");
    fprintf(output, "\tpush 0\n");
    fprintf(output, "\tcall main\n");
    fprintf(output, "\tpop\n");
    fprintf(output, "\thlt\n");
    fprintf(output, "\n");
}

static void _init_labels(FILE *output) {
    const size_t instr_size = 4;
    char *labels1[] = {"+", "-", "*", "/"};
    char *instrs1[] = {"add", "sub", "mul", "div"};
    for (size_t i = 0; i < instr_size; ++i) {
        fprintf(output, "label %s\n", labels1[i]);
        fprintf(output, "\tload $-3\n");
        fprintf(output, "\tload $-3\n");
        fprintf(output, "\t%s\n", instrs1[i]);
        fprintf(output, "\tstore $-4 $-1\n");
        fprintf(output, "\tpop\n");
        fprintf(output, "\tret\n");
        fprintf(output, "\n");
    }
    char *labels2[] = {"<", ">", "=", "/="};
    char *instrs2[] = {"jl", "jg", "je", "jne"};
    for (size_t i = 0; i < instr_size; ++i) {
        fprintf(output, "label %s\n", labels2[i]);
        fprintf(output, "\tload $-3\n");
        fprintf(output, "\tload $-3\n");
        fprintf(output, "\t%s %s_1\n", instrs2[i], labels2[i]);
        fprintf(output, "\tjmp %s_2\n", labels2[i]);
        fprintf(output, "label %s_1\n", labels2[i]);
        fprintf(output, "\tpush 1\n");
        fprintf(output, "\tjmp %s_end\n", labels2[i]);
        fprintf(output, "label %s_2\n", labels2[i]);
        fprintf(output, "\tpush 0\n");
        fprintf(output, "\tjmp %s_end\n", labels2[i]);
        fprintf(output, "label %s_end\n", labels2[i]);
        fprintf(output, "\tstore $-4 $-1\n");
        fprintf(output, "\tpop\n");
        fprintf(output, "\tret\n");
        fprintf(output, "\n");
    }
}

static int8_t _parse_code(FILE *output, FILE *input, size_t argc, HashTab *hashtab, char *proc) {
    int ch;
    while (isspace(ch = getc(input))) {
    }
    if (ch == EOF) {
        return 0;
    }
    if (ch != '(') {
        return 1;
    }
    HashTab *nhashtab = new_hashtab(25, STRING_TYPE, DECIMAL_TYPE);

    char operation[BUFF_SIZE];
    char buffer[BUFF_SIZE] = {0};

    _read_proc(input, buffer, BUFF_SIZE);
    int8_t op = _read_defop(buffer);

    memcpy(operation, buffer, BUFF_SIZE);

    switch(op){
        case DEFINE_INSTRC: 
            return _define_instrc(output, input, nhashtab, buffer, operation);
        case IF_INSTRC:
            return _if_instrc(output, input, nhashtab, hashtab, buffer, operation, proc, argc);
        default:
            return _proc_instrc(output, input, nhashtab, hashtab, buffer, operation, proc, argc);
    }

    free_hashtab(nhashtab);
    return -1;
}

static int8_t _define_instrc(
    FILE *output, 
    FILE *input, 
    HashTab *nhashtab, 
    char *buffer, 
    char *operation
) {
    int ch;

    _Bool proc_found = 0;
    _Bool proc_closed = 0;

    size_t args = 0;
    size_t index = 0;

    while((ch = getc(input)) != EOF) {
        if(isspace(ch)) {
            if (index != 0) {
                buffer[index] = '\0';
                index = 0;
                if (proc_found) {
                    set_hashtab(nhashtab, buffer, decimal(args));
                    args += 1;
                }
                if (!proc_found) {
                    memcpy(operation, buffer, BUFF_SIZE);
                    proc_found = 1;
                }
            }
            continue;
        }
        if (ch == '(') {
            continue;
        }
        if (ch == ')') {
            if (index != 0) {
                buffer[index] = '\0';
                index = 0;
                if (proc_found) {
                    set_hashtab(nhashtab, buffer, decimal(args));
                    args += 1;
                }
                if (!proc_found) {
                    memcpy(operation, buffer, BUFF_SIZE);
                    proc_found = 1;
                }
            }

            if (!proc_closed) {
                fprintf(output, "; args: %ld\n", args);
                fprintf(output, "label %s\n", operation);
                size_t temp = args;
                while (temp > 0) {
                    fprintf(output, "\tload $-%ld\n", 1+args);
                    temp -= 1;
                }
            }

            getc(input);
            _parse_code(output, input, args, nhashtab, operation);

            if (proc_closed) {
                free_hashtab(nhashtab);
                return 0;
            }

            fprintf(output, "label %s_end\n", operation);
            proc_closed = 1;
            if (args != 0) {
                fprintf(output, "\tstore $-%ld $-1\n", 2+(args*2));
            } else {
                fprintf(output, "\tstore $-%d $-1\n", 3);
            }

            fprintf(output, "\tpop\n");
            size_t temp = args;
            while (temp > 0) {
                fprintf(output, "\tpop\n");
                temp -= 1;
            }
            fprintf(output, "\tret\n");
            fprintf(output, "\n");
        }

        if (!proc_closed) {
            buffer[index++] = ch;
        }
    }
    free_hashtab(nhashtab);
    return 1;
}

static int8_t _if_instrc(
    FILE *output, 
    FILE *input, 
    HashTab *nhashtab, 
    HashTab *hashtab, 
    char *buffer, 
    char *operation, 
    char *proc, 
    size_t argc
) {
    int ch;

    size_t args = 0;
    uint8_t cond = 2;

    _Bool operation_found = 0;
    _Bool cond_closed = 0;

    size_t index = 0;
    counter += 2;

    size_t temp_counter = counter;

    while((ch = getc(input)) != EOF) {
        if(isspace(ch)) {
            if (index != 0) {
                buffer[index] = '\0';
                index = 0;
                if (operation_found && !cond_closed) {
                    if (in_hashtab(hashtab, buffer)) {
                        int32_t n = get_hashtab(hashtab, buffer).decimal;
                        fprintf(output, "\tload $-%ld\n", args+(argc-n));
                    } else {
                        fprintf(output, "\tpush %s\n", buffer);
                    }
                    args += 1;
                }
                if (!operation_found) {
                    memcpy(operation, buffer, BUFF_SIZE);
                    operation_found = 1;
                }
            }
            continue;
        }
        if (ch == '(') {
            if (cond_closed) {
                fprintf(output, "label _lbl_%ld\n", temp_counter-cond);
                cond -= 1;
            }
            ungetc(ch, input);
            _parse_code(output, input, argc, hashtab, proc);
            if (cond_closed) {
                fprintf(output, "\tjmp %s_end\n", proc);
            }
            if (!cond_closed) {
                fprintf(output, "\tpush 1\n");
                fprintf(output, "\tje _lbl_%ld\n", temp_counter-2);
                fprintf(output, "\tjmp _lbl_%ld\n", temp_counter-1);
                cond_closed = 1;
            }
            continue;
        }
        if (cond == 0) {
            free_hashtab(nhashtab);
            return 0;
        }
        buffer[index++] = ch;
    }
    free_hashtab(nhashtab);
    return 1;
}

static int8_t _proc_instrc(
    FILE *output, 
    FILE *input, 
    HashTab *nhashtab, 
    HashTab *hashtab, 
    char *buffer, 
    char *operation, 
    char *proc, 
    size_t argc
) {
    int ch;
    size_t args = 0;
    size_t index = 0;
    while((ch = getc(input)) != EOF) {
        if(isspace(ch)) {
            if (index != 0) {
                buffer[index] = '\0';
                if (in_hashtab(hashtab, buffer)) {
                    int32_t n = get_hashtab(hashtab, buffer).decimal;
                    fprintf(output, "\tload $-%ld\n", args+(argc-n));
                } else {
                    fprintf(output, "\tpush %s\n", buffer);
                }
                args += 1;
            }
            index = 0;
            continue;
        }
        if (ch == '(') {
            ungetc(ch, input);
            _parse_code(output, input, args+argc, hashtab, proc);
            args += 1;
            continue;
        }
        if (ch == ')') {
            if (index != 0) {
                buffer[index] = '\0';
                if (in_hashtab(hashtab, buffer)) {
                    int32_t n = get_hashtab(hashtab, buffer).decimal;
                    fprintf(output, "\tload $-%ld\n", args+(argc-n));
                } else {
                    fprintf(output, "\tpush %s\n", buffer);
                }
                args += 1;
            }
            if (args == 0) {
                fprintf(output, "\tpush 0\n");
            }
            fprintf(output, "\tcall %s\n", operation);
            while (args > 1) {
                fprintf(output, "\tpop\n");
                args -= 1;
            }
            free_hashtab(nhashtab);
            return 0;
        }
        buffer[index++] = ch;
    }
    free_hashtab(nhashtab);
    return 1;
}

static char *_read_proc(FILE *input, char *buffer, size_t size) {
    int ch = getc(input);
    while(isspace(ch)){
        ch = getc(input);
    }
    size_t i = 0;
    while(!isspace(ch) && i < size-1 && (ch != '(' && ch != ')')) {
        buffer[i++] = ch;
        ch = getc(input);
    }
    if (ch == '(' || ch == ')') {
        ungetc(ch, input);
    }
    return buffer;
}

static int8_t _read_defop(char *str) {
    for (int8_t i = 0; i < INSTRC_NUM; ++i) {
        if (strcmp(str, instructions[i]) == 0) {
            return i;
        }
    }
    return -1;
}
