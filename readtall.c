#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "extclib/stack.h"

#define STACK_SIZE 16
#define BUFF_SIZE 256
#define INSTRC_NUM 4

typedef enum  instrc_t {
    DEFINE_INSTRC,
    IF_INSTRC,
    INCLUDE_INSTRC,
    LOAD_INSTRC,
    PASS_INSTRC,
} instrc_t;

static const char *Instructions[INSTRC_NUM] = {
    [DEFINE_INSTRC]  = "define",
    [IF_INSTRC]      = "if",
    [INCLUDE_INSTRC] = "include",
    [LOAD_INSTRC]    = "load",
};

static void _init_entry(FILE *output);

static int8_t _parse_code(
    FILE *output, 
    FILE *input, 
    Stack *args, 
    char *root,
    size_t nesting,
    size_t counter
);
static int8_t _include_instrc(
    FILE *output, 
    FILE *input,
    char *buffer
);
static int8_t _load_instrc(
    FILE *output, 
    FILE *input,
    char *buffer
);
static int8_t _define_instrc(
    FILE *output, 
    FILE *input, 
    Stack *args, 
    char *buffer, 
    char *proc
);
static int8_t _if_instrc(
    FILE *output, 
    FILE *input, 
    Stack *args,
    char *root, 
    size_t nesting,
    size_t counter
);
static int8_t _proc_instrc(
    FILE *output, 
    FILE *input, 
    Stack *args, 
    char *buffer, 
    char *proc, 
    char *root, 
    size_t nesting,
    size_t counter
);

static int8_t _readtall_src(FILE *output, FILE *input);
static char *_read_proc(FILE *input, char *buffer, size_t size);
static instrc_t _read_defop(char *str);

extern int8_t readtall_src(FILE *output, FILE *input) {
    _init_entry(output);
    return _readtall_src(output, input);
}

static int8_t _readtall_src(FILE *output, FILE *input) {
    int ch;
    int8_t res = 0;
    while ((ch = getc(input)) != EOF) {
        ungetc(ch, input);
        res = _parse_code(output, input, NULL, "", 0, 0);
        if (res != 0) {
            return res;
        }
    }
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
    Stack *args, 
    char *root, 
    size_t nesting,
    size_t counter
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
    instrc_t op = _read_defop(buffer);

    memcpy(proc, buffer, BUFF_SIZE);

    switch(op){
        case DEFINE_INSTRC: {
            Stack *nargs = new_stack(STACK_SIZE, STRING_TYPE);
            int8_t res =_define_instrc(output, input, nargs, buffer, proc);
            free_stack(nargs);
            return res;
        }
        case IF_INSTRC:
            return _if_instrc(output, input, args, root, nesting, counter);
        case INCLUDE_INSTRC:
            return _include_instrc(output, input, buffer);
        case LOAD_INSTRC:
            return _load_instrc(output, input, buffer);
        default: 
            // printf("%s\t%s\t%ld\t%ld\n", root, proc, size_stack(args), nesting);
            return _proc_instrc(output, input, args, buffer, proc, root, nesting, counter);
    }

    return -1;
}

static int8_t _include_instrc(
    FILE *output, 
    FILE *input,
    char *buffer
) {
    int ch;
    size_t index = 0;

    while((ch = getc(input)) != EOF) {
        if (isspace(ch)) {
            if (index != 0) {
                buffer[index] = '\0';
                index = 0;
                FILE *file = fopen(buffer, "r");
                if (file == NULL) {
                    return 1;
                }
                _readtall_src(output, file);
                fclose(file);
            }
            continue;
        }
        if (ch == ';') {
            _pass_comment(ch, input);
            continue;
        }
        if (ch == ')') {
            if (index != 0) {
                buffer[index] = '\0';
                index = 0;
                FILE *file = fopen(buffer, "r");
                if (file == NULL) {
                    return 1;
                }
                _readtall_src(output, file);
                fclose(file);
            }
            break;
        }
        buffer[index++] = ch;
    }
    return 0;
}

static int8_t _load_instrc(
    FILE *output, 
    FILE *input,
    char *buffer
) {
    int ch;
    size_t index = 0;

    while((ch = getc(input)) != EOF) {
        if (isspace(ch)) {
            if (index != 0) {
                buffer[index] = '\0';
                index = 0;
                FILE *file = fopen(buffer, "r");
                if (file == NULL) {
                    return 1;
                }
                while(fgets(buffer, BUFF_SIZE, file) != NULL) {
                    fputs(buffer, output);
                }
                fclose(file);
            }
            continue;
        }
        if (ch == ';') {
            _pass_comment(ch, input);
            continue;
        }
        if (ch == ')') {
            if (index != 0) {
                buffer[index] = '\0';
                index = 0;
                FILE *file = fopen(buffer, "r");
                if (file == NULL) {
                    return 1;
                }
                while(fgets(buffer, BUFF_SIZE, file) != NULL) {
                    fputs(buffer, output);
                }
                fclose(file);
            }
            break;
        }
        buffer[index++] = ch;
    }
    return 0;
}

static int8_t _define_instrc_state(
    Stack *args, 
    char *buffer, 
    char *proc, 
    _Bool *proc_found,
    size_t *index
) {
    if (*index != 0) {
        buffer[*index] = '\0';
        *index = 0;
        if (*proc_found) {
            if (size_stack(args) == STACK_SIZE) {
                return 1;
            }
            push_stack(args, buffer);
        }
        if (!*proc_found) {
            memcpy(proc, buffer, BUFF_SIZE);
            *proc_found = 1;
        }
    }
    return 0;
}

static int8_t _define_instrc(
    FILE *output, 
    FILE *input, 
    Stack *args, 
    char *buffer, 
    char *proc
) {
    int ch;

    _Bool proc_found = 0;
    _Bool proc_closed = 0;

    size_t index = 0;

    while((ch = getc(input)) != EOF) {
        if (isspace(ch)) {
            if (_define_instrc_state(args, buffer, proc, &proc_found, &index)) {
                return 3;
            }
            continue;
        }
        if (ch == ';') {
            _pass_comment(ch, input);
            continue;
        }
        
        if (ch == '(') {
            if (proc_closed) {
                ungetc(ch, input);
                fprintf(output, "label _%s_begin\n", proc);
                _parse_code(output, input, args, proc, size_stack(args), 0);
            }
            continue;
        }

        if (ch == ')') {
            if (_define_instrc_state(args, buffer, proc, &proc_found, &index)) {
                return 3;
            }
            size_t argc = size_stack(args);

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

        if (proc_closed) {
            return 2;
        }

        buffer[index++] = ch;
    }

    return 1;
}

static int8_t _if_instrc(
    FILE *output, 
    FILE *input,
    Stack *args, 
    char *root, 
    size_t nesting,
    size_t counter
) {
    int ch;

    uint8_t cond_argc = 2;
    _Bool cond_closed = 0;

    while((ch = getc(input)) != EOF) {
        if (isspace(ch)) {
            continue;
        }
        if (ch == ';') {
            _pass_comment(ch, input);
            continue;
        }

        if (ch == '(') {
            size_t tcounter = counter + 2;

            if (cond_closed) {
                fprintf(output, "label _%s_%ld\n", root, tcounter-cond_argc);
                cond_argc -= 1;
            }

            ungetc(ch, input);
            _parse_code(output, input, args, root, nesting, tcounter);

            if (cond_closed && cond_argc == 1) {
                fprintf(output, "\tjmp _%s_end\n", root);
            }

            if (!cond_closed) {
                fprintf(output, "\tpush 0\n");
                fprintf(output, "\tjne _%s_%ld\n", root, tcounter-2);
                fprintf(output, "\tjmp _%s_%ld\n", root, tcounter-1);
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

static _Bool _in_stack(Stack *args, char *buffer, size_t *index) {
    for (int32_t i = 0; i < size_stack(args); ++i) {
        if (strcmp(get_stack(args, i).string, buffer) == 0) {
            *index = i;
            return 1;
        }
    }
    return 0;
}

static void _proc_instrc_state(
    FILE *output, 
    Stack *args, 
    char *buffer, 
    size_t nesting, 
    size_t *index, 
    size_t *argc_in
) {
    if (*index != 0) {
        buffer[*index] = '\0';
        *index = 0;
        size_t i;
        if (_in_stack(args, buffer, &i)) {
            fprintf(output, "\tload -%ld\n", *argc_in + nesting - i);
        } else {
            fprintf(output, "\tpush %s\n", buffer);
        }
        *argc_in += 1;
    }
}

static int8_t _proc_instrc(
    FILE *output, 
    FILE *input,
    Stack *args, 
    char *buffer, 
    char *proc, 
    char *root, 
    size_t nesting,
    size_t counter
) {
    int ch;

    size_t argc_in = 0;
    size_t index = 0;

    while((ch = getc(input)) != EOF) {
        if (isspace(ch)) {
            _proc_instrc_state(output, args, buffer, nesting, &index, &argc_in);
            continue;
        }
        if (ch == ';') {
            _pass_comment(ch, input);
            continue;
        }

        if (ch == '(') {
            ungetc(ch, input);
            _parse_code(output, input, args, root, argc_in + nesting, counter);
            argc_in += 1;
            continue;
        }

        if (ch == ')') {
            _proc_instrc_state(output, args, buffer, nesting, &index, &argc_in);

            if (argc_in == 0) {
                fprintf(output, "\tpush 0\n");
            }

            size_t i;
            if (_in_stack(args, proc, &i)) {
                fprintf(output, "\tcall -%ld\n", argc_in + nesting - i);
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

static instrc_t _read_defop(char *str) {
    for (int8_t i = 0; i < INSTRC_NUM; ++i) {
        if (strcmp(str, Instructions[i]) == 0) {
            return i;
        }
    }
    return PASS_INSTRC;
}
