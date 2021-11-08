#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "allkernel.h"
#include "CVM/extclib/type/list.h"

#define ALL_KERNEL_BSIZE 256
#define ALL_KERNEL_ISIZE 3

enum {
    OUT = 0,
    IN  = 1,
};

enum {
    I_DEFAULT = 0x00,
    I_DEFINE  = 0x01,
    I_INCLUDE = 0x02,
};

static struct instruction {
    uint8_t icode;
	char *iname;
} inlist[ALL_KERNEL_ISIZE] = {
    { I_DEFAULT, "\1"      },
    { I_DEFINE,  "define"  },
    { I_INCLUDE, "include" },
};

static int open_expr(FILE *output, FILE *input, list_t *ls, int argc);

static int compile_include(FILE *output, FILE *input);
static int compile_define(FILE *output, FILE *input, list_t *ls);
static int compile_default(FILE *output, FILE *input, char *name, list_t *ls, int argc);

static uint8_t read_icode(FILE *input, char *buffer);
static uint8_t find_icode(char *str);

static int curr_char(FILE *input);
static void file_trim_spaces(FILE *input);
static int file_read_word(FILE *input, char *buffer);

static uint16_t wrap_return(uint8_t x, uint8_t y);

// compile expressions
extern int all_compile(FILE *output, FILE *input) {
    int retcode;

    retcode = 0;

    while (1) {
        if (curr_char(input) == EOF) {
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
static int open_expr(FILE *output, FILE *input, list_t *ls, int argc) {
    char buffer[ALL_KERNEL_BSIZE];
    int retcode;

    list_t *newls;
    uint8_t icode;
    int state;
    int ch;

    state = OUT;

    while((ch = getc(input)) != EOF) {
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
            continue; 
        }

        ungetc(ch, input);
        icode = read_icode(input, buffer);

        switch(icode) {
            case I_INCLUDE:
                retcode = compile_include(output, input);
            break;
            case I_DEFINE:
                newls = list_new();
                retcode = compile_define(output, input, newls);
                list_free(newls);
            break;
            default:
                retcode = compile_default(output, input, buffer, ls, argc);
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
    char buffer[ALL_KERNEL_BSIZE];

    FILE *included;
    int len;
    
    while(1) {
        // read filepath of library
        len = file_read_word(input, buffer);
        if (len == 0) {
            if (curr_char(input) != ')') {
                return wrap_return(I_INCLUDE, 1);
            }
            break;
        }

        // open file of library
        included = fopen(buffer, "r");
        if (included == NULL) {
            return wrap_return(I_INCLUDE, 2);
        }

        // read and write lines
        // from library to compiled file
        while(fgets(buffer, ALL_KERNEL_BSIZE, included) != NULL) {
            fputs(buffer, output);
        }

        // close library
        fclose(included);
    }

    return 0;
}

// compile 'define' instruction
static int compile_define(FILE *output, FILE *input, list_t *ls) {
    char buffer[ALL_KERNEL_BSIZE];

    int argptr;
    int retcode;
    int count;
    int len;
    int ch;

    // check exists parenthesis symbol 
    // for function name
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
        if (curr_char(input) == '(') {
            return wrap_return(I_DEFINE, 3);
        }

        // read one arg
	    len = file_read_word(input, buffer);
        if (len == 0) {
            if (curr_char(input) != ')') {
                return wrap_return(I_DEFINE, 4);
            }
            // pass ')' symbol and break loop
            getc(input);
            break;
        }

        // append argument to list
        list_insert(ls, list_size(ls), buffer, len+1);
    }

    count = list_size(ls);

    // push arguments
    for (int i = 0; i < count; ++i) {
        fprintf(output, 
            "\tpush -%d\n"
            "\tload\n", 
                count+1);
    }

    // run body of function
    retcode = open_expr(output, input, ls, 0);
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
static int compile_default(FILE *output, FILE *input, char *name, list_t *ls, int argc) {
    char buffer[ALL_KERNEL_BSIZE];

    int argptr;
    int retcode;
    int count;
    int len;
    int i;

    count = 0;

    while(1) {
        // new expression into this
        if (curr_char(input) == '(') {
            retcode = open_expr(output, input, ls, count);
            if (retcode != 0) {
                return wrap_return(I_DEFAULT, retcode >> 8);
            }
            ++count;
            continue;
        }

        // read one argument 
	    len = file_read_word(input, buffer);
        if (len == 0) {
            if (curr_char(input) != ')') {
                return wrap_return(I_DEFAULT, 1);
            }
            break;
        }

        // save constant or
        // load variable from function input
        i = list_find(ls, buffer, len+1);
        if (i == -1) {
            fprintf(output, "\tpush %s\n", buffer);
        } else {
            argptr = list_size(ls) - i + argc + count;
            fprintf(output, 
                "\tpush -%d\n"
                "\tload\n", 
                    argptr);
        }

        ++count;
    }

    // if argc == 0 then push value
    // for get return result
    if (count == 0) {
        fprintf(output, "\tpush 0\n");
    }

    // get function
    i = list_find(ls, name, strlen(name)+1);
    if (i == -1) {
        // if global function
        fprintf(output, "\tpush %s\n", name);
    } else {
        // if function as argument in
        // another function
        argptr = list_size(ls) - i + argc + count;
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
		if (strcmp(buffer, inlist[i].iname) == 0) {
			icode = inlist[i].icode;
			break;
		}
	}

	return icode;
}

// return current char in file
static int curr_char(FILE *input) {
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