#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char** argv) {
    if (argc == 1) {
        puts("To compile a Brainfuck file using BFC. \n[PATH TO BFC] [OUTPUT "
             "NAME] [BRAINFUCK FILE]");
        return 1;
    } else if (argc != 3) {
        puts("\e[0;31mError:\e[m Not enough or too many arguments.");
        return 1;
    }

    char* cFileName = malloc(strlen(argv[2]) + 3);
    for (size_t i = 0; i <= strlen(argv[2]); i++) {
        if (argv[2][i] == '.') {
            cFileName[i] = '_';
            continue;
        }
        cFileName[i] = argv[2][i];
    }
    strcat(cFileName, ".c");

    // NOTE: The translated C file could've been written without using the
    // standard libraries, but dealing with inputs is a pain in the ass if we're
    // just using Assembly. Plus, who cares.

    FILE* cFile = fopen(cFileName, "w");
    cFile = freopen(cFileName, "a", cFile);
    fputs("#include <stdio.h>\nstatic unsigned char stack[30000];\nint main() "
          "{\n    unsigned char* ptr = stack;\n",
        cFile);

    FILE* bfFile = fopen(argv[2], "r");
    char ch = 0;
    size_t loopDepth = 0;
    while (ch != EOF) {
        ch = fgetc(bfFile);
        for (size_t i = 0; i < loopDepth; i++) { fputs("    ", cFile); }
        switch (ch) {
            case '>': fputs("    ptr++;\n", cFile); break;
            case '<': fputs("    ptr--;\n", cFile); break;
            case '+': fputs("    (*ptr)++;\n", cFile); break;
            case '-': fputs("    (*ptr)--;\n", cFile); break;
            case '.': fputs("    putchar(*ptr);\n", cFile); break;
            case ',': fputs("    *ptr = getchar();\n", cFile); break;
            case '[':
                fputs("    while (*ptr) {\n", cFile);
                loopDepth++;
                break;
            case ']':
                fputs("}\n", cFile);
                loopDepth--;
                break;
        }
    }

    fputc('}', cFile);
    fclose(cFile);

    pid_t runGCC = fork();
    if (runGCC == 0) {
        execl("/usr/bin/gcc", "gcc", "-o", argv[1], cFileName, NULL);
        exit(0);
    }
    remove(cFileName);
}
