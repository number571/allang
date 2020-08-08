#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define ERROR_NUM 6

typedef enum error_t {
    NONE_ERR,
    ARGLEN_ERR,
    COMMAND_ERR,
    INOPEN_ERR,
    OUTOPEN_ERR,
    TRANSLATE_ERR,
} error_t;

static const char *errors[ERROR_NUM] = {
    [NONE_ERR] = "",
    [ARGLEN_ERR] = "len argc < 3",
    [COMMAND_ERR] = "unknown command",
    [INOPEN_ERR] = "open input file",
    [OUTOPEN_ERR] = "open output file",
    [TRANSLATE_ERR] = "translate file",
};

extern int8_t readtall_src(FILE *output, FILE *input);

static void translate_src(const char *outputf, const char *inputf, int *retcode);
static char *help(void);

int main(int argc, char const *argv[]) {
	int retcode = NONE_ERR;

    if (argc < 3) {
        retcode = ARGLEN_ERR;
        goto close;
    } 

    if (strcmp(argv[1], "build") == 0) {
        if (argc >= 5 && strcmp(argv[3], "-o") == 0) {
            translate_src(argv[4], argv[2], &retcode);
        } else {
            translate_src("main.vms", argv[2], &retcode);
        }
        goto close;
    } 

    retcode = COMMAND_ERR;

close:
    if (retcode != NONE_ERR) {
        fprintf(stderr, "> %s\n%s", errors[retcode], help());
    }

    return retcode;
}

static char *help(void) {
    return \
    "BEGIN _Help_info_\n" \
    "\t1. Translate:\n" \
    "\t\t$ ctall build main.all\n" \
    "\t2. Compile and Run:\n" \
    "\t\t$ cvm build main.vms\n" \
    "\t\t$ cvm run main.vme\n" \
    "END _Help_info_\n";
}

static void translate_src(const char *outputf, const char *inputf, int *retcode) {
    FILE *input = fopen(inputf, "r");
    if (input == NULL) {
        *retcode = INOPEN_ERR;
        return;
    }
    FILE *output = fopen(outputf, "wb");
    if (output == NULL) {
        *retcode = OUTOPEN_ERR;
        return;
    }
    int8_t res = readtall_src(output, input);
    fclose(input);
    fclose(output);
    if (res != 0) {
        *retcode = TRANSLATE_ERR;
        return;
    }
}
