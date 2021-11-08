#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "allkernel.h"

#define ALL_HELP    "help"
#define ALL_BUILD   "build"
#define ALL_OUTFILE "main.vms"

enum {
    ERR_NONE    = 0x00,
    ERR_ARGLEN  = 0x01,
    ERR_COMMAND = 0x02,
    ERR_INOPEN  = 0x03,
    ERR_OUTOPEN = 0x04,
    ERR_COMPILE = 0x05,
};

static const char *errors[] = {
    [ERR_NONE]    = "",
    [ERR_ARGLEN]  = "len argc < 3",
    [ERR_COMMAND] = "unknown command",
    [ERR_INOPEN]  = "open input file",
    [ERR_OUTOPEN] = "open output file",
    [ERR_COMPILE] = "compile code",
};

static int file_build(const char *outputf, const char *inputf);

int main(int argc, char const *argv[]) {
    const char *outfile;

    int retcode;
    int is_build;

    outfile = ALL_OUTFILE;
    retcode = ERR_COMMAND;

    // all help
    if (argc == 2 && strcmp(argv[1], ALL_HELP) == 0) {
        printf("help: \n\t$ all build file [-o outfile]\n");
        return ERR_NONE;
    }

    // all | all undefined
    if (argc < 3) {
        fprintf(stderr, "error: %s\n", errors[ERR_ARGLEN]);
        return ERR_ARGLEN;
    }

    is_build = strcmp(argv[1], ALL_BUILD) == 0;

    // all undefined x
    if (!is_build) {
        fprintf(stderr, "error: %s\n", errors[ERR_COMMAND]);
        return ERR_COMMAND;
    }

    // all build file [-o outfile]
    if (is_build) {
        if (argc == 5 && strcmp(argv[3], "-o") == 0) {
            outfile = argv[4];
        }

        retcode = file_build(outfile, argv[2]);
        if (retcode != ERR_NONE) {
            fprintf(stderr, "error: %s\n", errors[retcode]);
        }
    }

    return retcode;
}

static int file_build(const char *outputf, const char *inputf) {
    FILE *output, *input;
    int retcode;

    input = fopen(inputf, "r");
    if (input == NULL) {
        return ERR_INOPEN;
    }

    output = fopen(outputf, "wb");
    if (output == NULL) {
        fclose(input);
        return ERR_OUTOPEN;
    }

    retcode = all_compile(output, input);

    fclose(input);
    fclose(output);
    
    if (retcode != ERR_NONE) {
        return ERR_COMPILE;
    }
    
    return ERR_NONE;
}
