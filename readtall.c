#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "extclib/type.h"

#define LIMIT_ARGS 16
#define INSTRC_NUM 4

typedef enum  instrc_t {
    INSTRC_DEFINE,
    INSTRC_IF,
    INSTRC_INCLUDE,
    INSTRC_LOAD,
    INSTRC_PASS,
} instrc_t;

static const char *instructions[INSTRC_NUM] = {
    [INSTRC_DEFINE]  = "define",
    [INSTRC_IF]      = "if",
    [INSTRC_INCLUDE] = "include",
    [INSTRC_LOAD]    = "load",
};

static void init_entry(FILE *output);

static int parse_code(
    FILE *output, 
    FILE *input, 
    type_list *args, 
    char *root,
    int nesting,
    int counter
);
static int instrc_include(
    FILE *output, 
    FILE *input,
    char *buffer
);
static int instrc_load(
    FILE *output, 
    FILE *input,
    char *buffer
);
static int instrc_define(
    FILE *output, 
    FILE *input, 
    type_list *args, 
    char *buffer, 
    char *proc
);
static int instrc_if(
    FILE *output, 
    FILE *input, 
    type_list *args,
    char *root, 
    int nesting,
    int counter
);
static int proc_expression(
    FILE *output, 
    FILE *input, 
    type_list *args, 
    char *buffer, 
    char *proc, 
    char *root, 
    int nesting,
    int counter
);

static int instrc_define_state(
    type_list *args, 
    char *buffer, 
    char *proc, 
    int *proc_found,
    int *index
);
static int start_read_src(FILE *output, FILE *input);
static char *read_proc(FILE *input, char *buffer, int size);
static instrc_t read_defop(char *str);
static int pass_comments_and_spaces(FILE *input);

extern int readtall_src(FILE *output, FILE *input) {
    init_entry(output);
    return start_read_src(output, input);
}

static int start_read_src(FILE *output, FILE *input) {
    int ch, res;
    res = 0;
    while ((ch = getc(input)) != EOF) {
        ungetc(ch, input);
        res = parse_code(output, input, NULL, "", 0, 0);
        if (res != 0) {
            return res;
        }
    }
    return res;
}

static void init_entry(FILE *output) {
    fprintf(output, "label _start\n");
    fprintf(output, "\tpush 0\n");
    fprintf(output, "\tcall main\n");
    fprintf(output, "\tpop\n");
    fprintf(output, "\thlt\n");
    fprintf(output, "\n");
}

static int pass_comments_and_spaces(FILE *input) {
    int ch;
    while ((ch = getc(input)) != EOF) {
        if (isspace(ch)) {
            continue;
        }
        if (ch == ';') {
            while(ch != '\n' && ch != EOF) {
                ch = getc(input);
            }
            continue;
        }
        break;
    }
    return ch;
}

static int parse_code(
    FILE *output, 
    FILE *input, 
    type_list *args, 
    char *root, 
    int nesting,
    int counter
) {
    type_list *nargs;
    int ch, res;
    char proc[BUFSIZ];
    char buffer[BUFSIZ];

    ch = pass_comments_and_spaces(input);

    if (ch == EOF) {
        return 0;
    }
    if (ch != '(') {
        return 1;
    }

    read_proc(input, buffer, BUFSIZ);
    instrc_t op = read_defop(buffer);

    memcpy(proc, buffer, BUFSIZ);

    switch(op){
        case INSTRC_DEFINE:
            nargs = type_list_new();
            res = instrc_define(output, input, nargs, buffer, proc);
            type_list_free(nargs);
            return res;
        case INSTRC_IF:
            return instrc_if(output, input, args, root, nesting, counter);
        case INSTRC_INCLUDE:
            return instrc_include(output, input, buffer);
        case INSTRC_LOAD:
            return instrc_load(output, input, buffer);
        default: 
            return proc_expression(output, input, args, buffer, proc, root, nesting, counter);
    }

    return -1;
}

static int instrc_define(
    FILE *output, 
    FILE *input, 
    type_list *args, 
    char *buffer, 
    char *proc
) {
    int ch, index, argc, temp;
    int proc_found, proc_closed;

    index = 0;
    proc_found = 0;
    proc_closed = 0;

    while((ch = getc(input)) != EOF) {
        if (ch == ';') {
            pass_comments_and_spaces(input);
            continue;
        }

        if (isspace(ch) || ch == ')') {
            if (index != 0 && instrc_define_state(args, buffer, proc, &proc_found, &index)) {
                return 3;
            }
            if (ch != ')') {
                continue;
            }
        }

        if (ch == ')') {
            argc = type_list_size(args);

            if (!proc_closed) {
                fprintf(output, "; argc: %d\n", argc);
                fprintf(output, "label %s\n", proc);

                temp = argc;
                while (temp > 0) {
                    fprintf(output, "\tload -%d\n", 1+argc);
                    temp -= 1;
                }

                fprintf(output, "\tjmp _%s_begin\n", proc);
                fprintf(output, "\n");
            }

            if (proc_closed) {
                fprintf(output, "label _%s_end\n", proc);
                
                if (argc != 0) {
                    fprintf(output, "\tstore -%d -1\n", 2+(argc*2));
                } else {
                    fprintf(output, "\tstore -%d -1\n", 3);
                }

                temp = argc+1;
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

        if (ch == '(') {
            if (!proc_closed) {
                continue;
            }
            ungetc(ch, input);
            fprintf(output, "label _%s_begin\n", proc);
            parse_code(output, input, args, proc, type_list_size(args), 0);
            continue;
        }

        buffer[index++] = ch;
    }

    return 1;
}

static int instrc_define_state(
    type_list *args, 
    char *buffer, 
    char *proc, 
    int *proc_found,
    int *index
) {
    buffer[*index] = '\0';
    *index = 0;
    if (*proc_found) {
        if (type_list_size(args) == LIMIT_ARGS) {
            return 1;
        }
        type_list_insert(args, type_list_size(args), buffer, strlen(buffer)+1);
    } else {
        memcpy(proc, buffer, BUFSIZ);
        *proc_found = 1;
    }
    return 0;
}

static int instrc_include(
    FILE *output, 
    FILE *input,
    char *buffer
) {
    FILE *file;
    int ch, index;

    index = 0;

    while((ch = getc(input)) != EOF) {
        if (ch == ';') {
            pass_comments_and_spaces(input);
            continue;
        }

        if (isspace(ch) || ch == ')') {
            if (index == 0) {
                continue;
            }
            buffer[index] = '\0';
            index = 0;
            file = fopen(buffer, "r");
            if (file == NULL) {
                return 1;
            }
            start_read_src(output, file);
            fclose(file);
            if (ch == ')') {
                break;
            }
            continue;
        }
        
        buffer[index++] = ch;
    }

    return 0;
}

static int instrc_load(
    FILE *output, 
    FILE *input,
    char *buffer
) {
    FILE *file;
    int ch, index;
    
    index = 0;

    while((ch = getc(input)) != EOF) {
        if (ch == ';') {
            pass_comments_and_spaces(input);
            continue;
        }

        if (isspace(ch) || ch == ')') {
            if (index == 0) {
                continue;
            }
            buffer[index] = '\0';
            index = 0;
            file = fopen(buffer, "r");
            if (file == NULL) {
                return 1;
            }
            while(fgets(buffer, BUFSIZ, file) != NULL) {
                fputs(buffer, output);
            }
            fclose(file);
            if (ch == ')') {
                break;
            }
            continue;
        }
        
        buffer[index++] = ch;
    }
    return 0;
}

static int instrc_if(
    FILE *output, 
    FILE *input,
    type_list *args, 
    char *root, 
    int nesting,
    int counter
) {
    int ch, tcounter;
    int cond_argc, cond_closed;

    cond_argc = 2;
    cond_closed = 0;

    while((ch = getc(input)) != EOF) {
        if (cond_argc == 0) {
            return 0;
        }

        if (isspace(ch)) {
            continue;
        }
        if (ch == ';') {
            pass_comments_and_spaces(input);
            continue;
        }

        if (ch == '(') {
            tcounter = counter + 2;

            if (cond_closed) {
                fprintf(output, "label _%s_%d\n", root, tcounter-cond_argc);
                cond_argc -= 1;
            }

            ungetc(ch, input);
            parse_code(output, input, args, root, nesting, tcounter);

            if (cond_closed && cond_argc == 1) {
                fprintf(output, "\tjmp _%s_end\n", root);
            }

            if (!cond_closed) {
                fprintf(output, "\tpush 0\n");
                fprintf(output, "\tjne _%s_%d\n", root, tcounter-2);
                fprintf(output, "\tjmp _%s_%d\n", root, tcounter-1);
                cond_closed = 1;
            }
            continue;
        }
    }

    return 1;
}

static void proc_expression_state(
    FILE *output, 
    type_list *args, 
    char *buffer, 
    int nesting, 
    int *index, 
    int *argc_in
) {
    int i;
    if (*index == 0) {
        return;
    }
    buffer[*index] = '\0';
    *index = 0;
    i = type_list_find(args, buffer, strlen(buffer)+1);
    if (i != -1) {
        fprintf(output, "\tload -%d\n", *argc_in + nesting - i);
    } else {
        fprintf(output, "\tpush %s\n", buffer);
    }
    *argc_in += 1;
}

static int proc_expression(
    FILE *output, 
    FILE *input,
    type_list *args, 
    char *buffer, 
    char *proc, 
    char *root, 
    int nesting,
    int counter
) {
    int ch, argc_in;
    int i, index;
    
    argc_in = 0;
    index = 0;

    while((ch = getc(input)) != EOF) {
        if (isspace(ch)) {
            proc_expression_state(output, args, buffer, nesting, &index, &argc_in);
            continue;
        }
        if (ch == ';') {
            pass_comments_and_spaces(input);
            continue;
        }

        if (ch == '(') {
            ungetc(ch, input);
            parse_code(output, input, args, root, argc_in + nesting, counter);
            argc_in += 1;
            continue;
        }

        if (ch == ')') {
            proc_expression_state(output, args, buffer, nesting, &index, &argc_in);

            if (argc_in == 0) {
                fprintf(output, "\tpush 0\n");
            }

            i = type_list_find(args, proc, strlen(proc)+1);
            if (i != -1) {
                fprintf(output, "\tcall -%d\n", argc_in + nesting - i);
            } else {
                if (strlen(proc) == 0) {
                    fprintf(output, "\tcall -%d\n", argc_in);
                    fprintf(output, "\tstore -%d -%d\n", argc_in, 
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

static char *read_proc(FILE *input, char *buffer, int size) {
    int ch, i;

    i = 0;
    ch = getc(input);

    while(!isspace(ch) && i < size-1 && (ch != '(' && ch != ')')) {
        buffer[i++] = ch;
        ch = getc(input);
    }

    if (ch == '(' || ch == ')') {
        ungetc(ch, input);
    }

    buffer[i] = '\0';
    return buffer;
}

static instrc_t read_defop(char *str) {
    for (int i = 0; i < INSTRC_NUM; ++i) {
        if (strcmp(str, instructions[i]) == 0) {
            return i;
        }
    }
    return INSTRC_PASS;
}
