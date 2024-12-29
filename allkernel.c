#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "allkernel.h"

#include "cvm/typeslib/hashtab.h"
#include "cvm/typeslib/list.h"

#define ALL_KERNEL_ISIZE 4

enum {
    I_DEFAULT = 0x00,
    I_INCLUDE = 0x01,
    I_IF      = 0x02,
    I_DEFINE  = 0x03,
};

static hashtab_t *libraries;
static uint32_t iternumber;

static struct instruction {
    uint8_t icode;
    char *iname;
} ilist[ALL_KERNEL_ISIZE] = {
    { I_DEFAULT, "\1"      },
    { I_INCLUDE, "include" },
    { I_IF,      "if"      },
    { I_DEFINE,  "define"  },
};

static int start_compile(FILE *output, FILE *input);
static int open_expr(FILE *output, FILE *input, list_t *args, int currc);

static int compile_include(FILE *output, FILE *input);
static int compile_if(FILE *output, FILE *input, list_t *args, int currc);
static int compile_define(FILE *output, FILE *input, list_t *args);
static int compile_default(FILE *output, FILE *input, list_t *args, int currc, char *op);

static uint8_t read_icode(FILE *input, char *buffer);
static uint8_t find_icode(char *str);

static void file_copy_text(FILE *output, FILE *input);
static void file_read_while(FILE *input, char fch);
static int file_curr_char(FILE *input);
static void file_trim_spaces(FILE *input);
static int file_read_word(FILE *input, char *buffer);

static uint16_t wrap_return(uint8_t x, uint8_t y);

// initialize function
extern int all_compile(FILE *output, FILE *input) {
    int retcode;

    iternumber = 0;
    libraries = hashtab_new(256);

    retcode = start_compile(output, input);

    hashtab_free(libraries);

    return retcode;
}

// compile expressions
static int start_compile(FILE *output, FILE *input) {
    int retcode;

    retcode = 0;

    while (1) {
        if (file_curr_char(input) == EOF) {
            break;
        }

        retcode = open_expr(output, input, NULL, 0);
        if (retcode != 0) {
            break;
        }
    }

    return retcode;
}

// open expression for check '(' and ')' chars
// and route to instructions
static int open_expr(FILE *output, FILE *input, list_t *args, int currc) {
    enum {
        OUT = 0,
        IN  = 1,
    } state;

    char buffer[BUFSIZ];
    int retcode;

    list_t *newls;
    uint8_t icode;
    int ch;

    state = OUT;

    while((ch = getc(input)) != EOF) {
        // if comment then pass line
        if (ch == ';') {
            file_read_while(input, '\n');
            continue;
        }

        // state -> IN
        if (ch == '(') {
            if (state == IN) {
                return wrap_return(0x00, 2);
            }
            state = IN;
            continue;
        }

        // state -> OUT
        if (ch == ')') {
            if (state == OUT) {
                return wrap_return(0x00, 1);
            }
            state = OUT;
            break;
        }

        // actions possible if state = IN
        if (state == OUT) {
            if (!isspace(ch)) {
                return wrap_return(0x00, 3);
            }
            continue; 
        }

        ungetc(ch, input);
        icode = read_icode(input, buffer);

        switch(icode) {
            case I_INCLUDE:
                retcode = compile_include(output, input);
            break;
            case I_IF:
                retcode = compile_if(output, input, args, currc);
            break;
            case I_DEFINE:
                newls = list_new();
                retcode = compile_define(output, input, newls);
                list_free(newls);
            break;
            default:
                retcode = compile_default(output, input, args, currc, buffer);
            break;
        }

        if (retcode != 0) {
            return retcode;
        }
    }

    return 0;
}

// include compiled code into process of compiling
static int compile_include(FILE *output, FILE *input) {
    char buffer[BUFSIZ];

    FILE *included;
    char *exists;
    int len;

    int is_vms;
    int is_all;

    // type of include files [assembly|source]
    len = file_read_word(input, buffer);
    if (len == 0) {
        return wrap_return(I_INCLUDE, 1);
    }

    is_vms = strcmp(buffer, "assembly") == 0;
    is_all = strcmp(buffer, "source") == 0;

    if (!is_vms && !is_all) {
        return wrap_return(I_INCLUDE, 2);
    }

    while(1) {
        // read filepath of library
        len = file_read_word(input, buffer);
        if (len == 0) {
            if (file_curr_char(input) != ')') {
                return wrap_return(I_INCLUDE, 3);
            }
            break;
        }

        // check if library already imported
        exists = hashtab_select(libraries, buffer);
        if (exists) {
            continue;
        } 

        // save library to storage
        hashtab_insert(libraries, buffer, "\1", 1);

        // open file of library
        included = fopen(buffer, "r");
        if (included == NULL) {
            return wrap_return(I_INCLUDE, 4);
        }

        // if assembly file then just copy this code
        if (is_vms) {
            file_copy_text(output, included);
        }

        // if source file then compile this to assembly code
        if (is_all) {
            start_compile(output, included);
        }

        // close library
        fclose(included);
    }

    return 0;
}

// compile 'if' instruction
static int compile_if(FILE *output, FILE *input, list_t *args, int currc) {
    int retcode;
    uint32_t curriter;

    curriter = iternumber++;

    // block condition
    retcode = open_expr(output, input, args, currc);
    if (retcode != 0) {
        return wrap_return(I_IF, 1);
    }

    fprintf(output, 
        "\tpush 0\n"
        "\tpush _if_%d\n"
        "\tjg\n"
        "\tpush _else_%d\n"
        "\tjmp\n"
        "labl _if_%d\n",
            curriter,
            curriter,
            curriter);

    // block if
    retcode = open_expr(output, input, args, currc);
    if (retcode != 0) {
        return wrap_return(I_IF, 2);
    }

    fprintf(output, 
        "\tpush _end_%d\n"
        "\tjmp\n"
        "labl _else_%d\n",
            curriter,
            curriter);

    // block else
    retcode = open_expr(output, input, args, currc);
    if (retcode != 0) {
        return wrap_return(I_IF, 3);
    }

    // end condition
    fprintf(output, "labl _end_%d\n", curriter);

    return 0;
}

// compile 'define' instruction
static int compile_define(FILE *output, FILE *input, list_t *args) {
    char buffer[BUFSIZ];

    int argptr;
    int retcode;
    int count;
    int len;
    int ch;

    // check exists parenthesis symbol 
    // for function name and pass this
    file_trim_spaces(input);
    ch = getc(input);
    if (ch != '(') {
        return wrap_return(I_DEFINE, 1);
    }

    // get function name
    len = file_read_word(input, buffer);
    if (len == 0) {
        return wrap_return(I_DEFINE, 2);
    }

    // put function name as label
    fprintf(output, "labl %s\n", buffer);

    // parse arguments
    while(1) {
        // in args can't be exists '('
        if (file_curr_char(input) == '(') {
            return wrap_return(I_DEFINE, 3);
        }

        // read one arg
        len = file_read_word(input, buffer);
        if (len == 0) {
            if (file_curr_char(input) != ')') {
                return wrap_return(I_DEFINE, 4);
            }
            // pass ')' symbol and break loop
            getc(input);
            break;
        }

        // append argument to list
        list_insert(args, list_size(args), buffer, len+1);
    }

    count = list_size(args);

    // push arguments
    for (int i = 0; i < count; ++i) {
        fprintf(output, 
            "\tpush -%d\n"
            "\tload\n", 
                count+1);
    }

    // run body of function
    retcode = open_expr(output, input, args, 0);
    if (retcode != 0) {
        return wrap_return(I_DEFINE, retcode >> 8);
    }

    // return result
    argptr = (count * 2) - (count ? 1 : 0) + 3;
    fprintf(output,
        "\tpush -1\n"
        "\tpush -%d\n"
        "\tstor\n",
            argptr);

    // free stack
    for (int i = 0; i < count+1; ++i) {
        fprintf(output, "\tpop\n");
    }

    // ret
    fprintf(output, "\tjmp\n");

    return 0;
}

// compile default instructions
static int compile_default(FILE *output, FILE *input, list_t *args, int currc, char *op) {
    char buffer[BUFSIZ];

    int argptr;
    int retcode;
    int count;
    int len;
    int i;

    for(count = 0 ;; ++count) {
        // read one argument 
        len = file_read_word(input, buffer);
        if (len == 0) {
            // new expression into this
            if (file_curr_char(input) == '(') {
                retcode = open_expr(output, input, args, currc+count);
                if (retcode != 0) {
                    return wrap_return(I_DEFAULT, retcode >> 8);
                }
                continue;
            }

            // end of expression
            if (file_curr_char(input) == ')') {
                break;
            }

            // another char is anomaly 
            return wrap_return(I_DEFAULT, 1);
        }

        // save constant or
        // load variable from function input
        i = list_find(args, buffer, len+1);
        if (i == -1) {
            fprintf(output, "\tpush %s\n", buffer);
        } else {
            argptr = list_size(args) - i + currc + count;
            fprintf(output, 
                "\tpush -%d\n"
                "\tload\n", 
                    argptr);
        }
    }

    // if argc == 0 then push value
    // for get return result
    if (count == 0) {
        count += 1;
        fprintf(output, "\tpush 0\n");
    }

    // get function
    i = list_find(args, op, strlen(op)+1);
    if (i == -1) {
        // if global function
        fprintf(output, "\tpush %s\n", op);
    } else {
        // if function as argument in
        // another function
        argptr = list_size(args) - i + currc + count;
        fprintf(output, 
            "\tpush -%d\n"
            "\tload\n", 
                argptr);
    }

    // run action of function
    fprintf(output, "\tcall\n");

    // free stack
    for (int i = 0; i < count-1; ++i) {
        fprintf(output, "\tpop\n");
    }

    return 0;
}

// get instruction code with name from file
static uint8_t read_icode(FILE *input, char *buffer) {
    file_read_word(input, buffer);
    return find_icode(buffer);
}

// get instruction code by name
// example: "define" -> I_DEFINE
static uint8_t find_icode(char *buffer) {
    uint8_t icode;
    icode = I_DEFAULT;

    for (int i = 0; i < ALL_KERNEL_ISIZE; ++i) {
        if (strcmp(buffer, ilist[i].iname) == 0) {
            icode = ilist[i].icode;
            break;
        }
    }

    return icode;
}

// copy file line by line
static void file_copy_text(FILE *output, FILE *input) {
    static char buffer[BUFSIZ];
    while(fgets(buffer, BUFSIZ, input) != NULL) {
        fputs(buffer, output);
    }
}

// read chars from file while char not
// equals 'fch'
static void file_read_while(FILE *input, char fch) {
    int ch;
    while ((ch = getc(input)) != EOF) {
        if (ch == fch) {
            break;
        }
    }
}

// return current char in file
static int file_curr_char(FILE *input) {
    int ch;
    ungetc(ch = getc(input), input);
    return ch;
}

// example: "  word1 word2 word3" -> "word1 word2 word3"
static void file_trim_spaces(FILE *input) {
    int ch; 
    ch = ' ';

    while(isspace(ch)) {
        ch = getc(input);
    }
    
    ungetc(ch, input);
}

// read word from file while char is not '(' or ')' or [space]
static int file_read_word(FILE *input, char *buffer) {
    int ch; 
    int i;

    i = 0;
    file_trim_spaces(input);

    while((ch = getc(input)) != EOF) {
        if (ch == '(' || ch == ')') {
            ungetc(ch, input);
            buffer[i] = '\0';
            break;
        }
        if (isspace(ch)) {
            buffer[i] = '\0';
            break;
        }
        buffer[i++] = ch; 
    }

    return i;
}

// return (x || y)
static uint16_t wrap_return(uint8_t x, uint8_t y) {
    return ((uint16_t)x << 8) | y;
}
