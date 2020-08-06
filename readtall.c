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

static size_t labels_counter = 0;
static const char *instructions[INSTRC_NUM] = {
    [DEFINE_INSTRC]    = "define",
    [IF_INSTRC]        = "if",
};

static void _init_entry(FILE *output);
static void _init_labels(FILE *output);

static int8_t _parse_code(
    FILE *output, 
    FILE *input, 
    HashTab *hashtab, 
    char *proc,
    size_t argc, 
    _Bool begin_exist
);

static int8_t _define_instrc(
    FILE *output, 
    FILE *input, 
    HashTab *hashtab, 
    char *buffer, 
    char *proc
);
static int8_t _if_instrc(
    FILE *output, 
    FILE *input, 
    HashTab *hashtab,
    char *root, 
    size_t argc
);
static int8_t _proc_instrc(
    FILE *output, 
    FILE *input, 
    HashTab *hashtab, 
    char *buffer, 
    char *proc, 
    char *root, 
    size_t argc
);

static char *_read_proc(FILE *input, char *buffer, size_t size);
static int8_t _read_defop(char *str);

extern int8_t readtall_src(FILE *output, FILE *input) {
    _init_entry(output);
    int ch;
    int res = 0;
    while ((ch = getc(input)) != EOF) {
        ungetc(ch, input);
        res = _parse_code(output, input, NULL, "", 0, 0);
        if (res != 0) {
            return res;
        }
    }
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
    fprintf(output, "label return\n");
    fprintf(output, "\tload -2\n");
    fprintf(output, "\tstore -3 -1\n");
    fprintf(output, "\tpop\n");
    fprintf(output, "\tret\n");
    fprintf(output, "\n");
    const size_t instr_size = 4;
    char *labels1[] = {"+", "-", "*", "/"};
    char *instrs1[] = {"add", "sub", "mul", "div"};
    for (size_t i = 0; i < instr_size; ++i) {
        fprintf(output, "label %s\n", labels1[i]);
        fprintf(output, "\tload -3\n");
        fprintf(output, "\tload -3\n");
        fprintf(output, "\t%s\n", instrs1[i]);
        fprintf(output, "\tstore -4 -1\n");
        fprintf(output, "\tpop\n");
        fprintf(output, "\tret\n");
        fprintf(output, "\n");
    }
    char *labels2[] = {"<", ">", "=", "/="};
    char *instrs2[] = {"jl", "jg", "je", "jne"};
    for (size_t i = 0; i < instr_size; ++i) {
        labels_counter += 2;
        fprintf(output, "label %s\n", labels2[i]);
        fprintf(output, "\tload -3\n");
        fprintf(output, "\tload -3\n");
        fprintf(output, "\t%s _lbl_%ld\n", instrs2[i], labels_counter-2);
        fprintf(output, "\tjmp _lbl_%ld\n", labels_counter-1);
        fprintf(output, "label _lbl_%ld\n", labels_counter-2);
        fprintf(output, "\tpush 1\n");
        fprintf(output, "\tjmp _%s_end\n", labels2[i]);
        fprintf(output, "label _lbl_%ld\n", labels_counter-1);
        fprintf(output, "\tpush 0\n");
        fprintf(output, "\tjmp _%s_end\n", labels2[i]);
        fprintf(output, "label _%s_end\n", labels2[i]);
        fprintf(output, "\tstore -4 -1\n");
        fprintf(output, "\tpop\n");
        fprintf(output, "\tret\n");
        fprintf(output, "\n");
    }
}

static int _pass_comment(int ch, FILE *input) {
    if (ch == ';') {
        while(ch != '\n' && ch != EOF) {
            ch = getc(input);
        }
    }
    return ch;
}

static int _pass_comments_spaces(FILE *input) {
    int ch;
    while ((ch = getc(input)) != EOF) {
        if (isspace(ch)) {
            continue;
        }
        if (ch == ';') {
            _pass_comment(ch, input);
            continue;
        }
        break;
    }
    return ch;
}

static int8_t _parse_code(
    FILE *output, 
    FILE *input, 
    HashTab *hashtab, 
    char *root, 
    size_t argc,
    _Bool begin_exist
) {
    int ch = _pass_comments_spaces(input);

    if (ch == EOF) {
        return 0;
    }
    if (ch != '(') {
        return 1;
    }

    char proc[BUFF_SIZE];
    char buffer[BUFF_SIZE] = {0};

    _read_proc(input, buffer, BUFF_SIZE);
    int8_t op = _read_defop(buffer);

    memcpy(proc, buffer, BUFF_SIZE);

    if (op != DEFINE_INSTRC && !begin_exist) {
        fprintf(output, "label _%s_begin\n", root);
    }

    switch(op){
        case DEFINE_INSTRC: {
            HashTab *nhashtab = new_hashtab(25, STRING_TYPE, DECIMAL_TYPE);
            int8_t res =_define_instrc(output, input, nhashtab, buffer, proc);
            free_hashtab(nhashtab);
            return res;
        }
        case IF_INSTRC:
            return _if_instrc(output, input, hashtab, root, argc);
        default: 
            return _proc_instrc(output, input, hashtab, buffer, proc, root, argc);
    }

    return -1;
}

static void _define_instrc_state(
    HashTab *hashtab, 
    char *buffer, 
    char *proc, 
    _Bool *proc_found,
    size_t *index, 
    size_t *argc
) {
    if (*index != 0) {
        buffer[*index] = '\0';
        *index = 0;
        if (*proc_found) {
            set_hashtab(hashtab, buffer, decimal(*argc));
            *argc += 1;
        }
        if (!*proc_found) {
            memcpy(proc, buffer, BUFF_SIZE);
            *proc_found = 1;
        }
    }
}

static int8_t _define_instrc(
    FILE *output, 
    FILE *input, 
    HashTab *hashtab, 
    char *buffer, 
    char *proc
) {
    int ch;

    _Bool proc_found = 0;
    _Bool proc_closed = 0;

    size_t argc = 0;
    size_t index = 0;

    while((ch = getc(input)) != EOF) {
        if (isspace(ch)) {
            _define_instrc_state(hashtab, buffer, proc, &proc_found, &index, &argc);
            continue;
        }
        if (ch == ';') {
            _pass_comment(ch, input);
            continue;
        }
        
        if (ch == '(') {
            if (proc_closed) {
                ungetc(ch, input);
                _parse_code(output, input, hashtab, proc, argc, 0);
            }
            continue;
        }

        if (ch == ')') {
            _define_instrc_state(hashtab, buffer, proc, &proc_found, &index, &argc);

            if (!proc_closed) {
                fprintf(output, "; argc: %ld\n", argc);
                fprintf(output, "label %s\n", proc);

                size_t temp = argc;
                while (temp > 0) {
                    fprintf(output, "\tload -%ld\n", 1+argc);
                    temp -= 1;
                }

                fprintf(output, "\tjmp _%s_begin\n", proc);
                fprintf(output, "\n");
            }

            if (proc_closed) {
                fprintf(output, "label _%s_end\n", proc);
                
                if (argc != 0) {
                    fprintf(output, "\tstore -%ld -1\n", 2+(argc*2));
                } else {
                    fprintf(output, "\tstore -%d -1\n", 3);
                }

                fprintf(output, "\tpop\n");
                size_t temp = argc;

                while (temp > 0) {
                    fprintf(output, "\tpop\n");
                    temp -= 1;
                }

                fprintf(output, "\tret\n");
                fprintf(output, "\n");

                return 0;
            }

            proc_closed = 1;
            continue;
        }

        if (!proc_closed) {
            buffer[index++] = ch;
        } else {
            return 2;
        }
    }

    return 1;
}

static int8_t _if_instrc(
    FILE *output, 
    FILE *input,
    HashTab *hashtab, 
    char *root, 
    size_t argc
) {
    int ch;

    uint8_t cond_argc = 2;
    _Bool cond_closed = 0;

    labels_counter += 2;
    size_t tcounter = labels_counter;

    while((ch = getc(input)) != EOF) {
        if (isspace(ch)) {
            continue;
        }
        if (ch == ';') {
            _pass_comment(ch, input);
            continue;
        }

        if (ch == '(') {
            if (cond_closed) {
                fprintf(output, "label _lbl_%ld\n", tcounter-cond_argc);
                cond_argc -= 1;
            }

            ungetc(ch, input);
            _parse_code(output, input, hashtab, root, argc, 1);

            if (cond_closed) {
                fprintf(output, "\tjmp _%s_end\n", root);
            }

            if (!cond_closed) {
                fprintf(output, "\tpush 0\n");
                fprintf(output, "\tjne _lbl_%ld\n", tcounter-2);
                fprintf(output, "\tjmp _lbl_%ld\n", tcounter-1);
                cond_closed = 1;
            }
            continue;
        }

        if (cond_argc == 0) {
            return 0;
        }
    }

    return 1;
}

static void _proc_instrc_state(
    FILE *output, 
    HashTab *hashtab, 
    char *buffer, 
    size_t *index, 
    size_t *argc, 
    size_t *argc_in
) {
    if (*index != 0) {
        buffer[*index] = '\0';
        *index = 0;
        if (in_hashtab(hashtab, buffer)) {
            int32_t n = get_hashtab(hashtab, buffer).decimal;
            fprintf(output, "\tload -%ld\n", *argc_in+(*argc-n));
        } else {
            fprintf(output, "\tpush %s\n", buffer);
        }
        *argc_in += 1;
    }
}

static int8_t _proc_instrc(
    FILE *output, 
    FILE *input,
    HashTab *hashtab, 
    char *buffer, 
    char *proc, 
    char *root, 
    size_t argc
) {
    int ch;

    size_t argc_in = 0;
    size_t index = 0;

    while((ch = getc(input)) != EOF) {
        if (isspace(ch)) {
            _proc_instrc_state(output, hashtab, buffer, &index, &argc, &argc_in);
            continue;
        }
        if (ch == ';') {
            _pass_comment(ch, input);
            continue;
        }

        if (ch == '(') {
            ungetc(ch, input);
            _parse_code(output, input, hashtab, root, argc_in+argc, 1);
            argc_in += 1;
            continue;
        }

        if (ch == ')') {
            _proc_instrc_state(output, hashtab, buffer, &index, &argc, &argc_in);

            if (argc_in == 0) {
                fprintf(output, "\tpush 0\n");
            }

            if (in_hashtab(hashtab, proc)) {
                int32_t n = get_hashtab(hashtab, proc).decimal;
                fprintf(output, "\tcall -%ld\n", argc_in+(argc-n));
            } else {
                if (strlen(proc) == 0) {
                    fprintf(output, "\tcall -%ld\n", argc_in);
                    fprintf(output, "\tstore -%ld -%ld\n", argc_in, 
                        (argc_in <= 1) ? 1 : (argc_in-1));
                } else {
                    fprintf(output, "\tcall %s\n", proc);
                }
            }

            while (argc_in > 1) {
                fprintf(output, "\tpop\n");
                argc_in -= 1;
            }

            return 0;
        }

        buffer[index++] = ch;
    }

    return 1;
}

static char *_read_proc(FILE *input, char *buffer, size_t size) {
    int ch = _pass_comments_spaces(input);
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
