#include <stdio.h>
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

extern int readtall_src(FILE *output, FILE *input);

static int translate_src(const char *outputf, const char *inputf);
static char *help(void);

int main(int argc, char const *argv[]) {
	int retcode = NONE_ERR;

    if (argc < 3) {
        retcode = ARGLEN_ERR;
        goto close;
    } 

    if (strcmp(argv[1], "build") == 0) {
        if (argc >= 5 && strcmp(argv[3], "-o") == 0) {
            retcode = translate_src(argv[4], argv[2]);
        } else {
            retcode = translate_src("main.vms", argv[2]);
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

static int translate_src(const char *outputf, const char *inputf) {
    FILE *input = fopen(inputf, "r");
    if (input == NULL) {
        return INOPEN_ERR;
    }
    FILE *output = fopen(outputf, "wb");
    if (output == NULL) {
        return OUTOPEN_ERR;
    }
    int res = readtall_src(output, input);
    fclose(input);
    fclose(output);
    if (res != 0) {
        return TRANSLATE_ERR;
    }
    return NONE_ERR;
}
