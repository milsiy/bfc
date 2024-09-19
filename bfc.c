#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char** argv) {
    if (argc == 1) {
        puts(
            "To compile a Brainfuck file using BFC. \n[PATH TO BFC] [BRAINFUCK "
            "FILE]");
        return 1;
    } else if (argc != 2) {
        puts("\033[0;31mError:\033[m Not enough or too many arguments.");
        return 1;
    }

    char* cFileName = malloc(strlen(argv[1]) + 3);
    strcpy(cFileName, argv[1]);
    strcat(cFileName, ".c");

    char* outFileName = malloc(strlen(argv[1]) - 2);
    strncpy(outFileName, argv[1], strlen(argv[1]) - 3);
    outFileName[strlen(argv[1]) - 3] = 0;

    // NOTE: The translated C file could've been written without using the
    // standard libraries, but dealing with inputs is a pain in the ass if we're
    // just using Assembly. Plus, who cares.

    FILE* cFile = fopen(cFileName, "w+a");
    fputs(
        "#include <stdio.h>\nstatic unsigned char stack[30000];int "
        "main(){unsigned char*ptr=stack;",
        cFile);

    FILE*   bfFile    = fopen(argv[1], "r");
    char    ch        = 0;
    ssize_t loopDepth = 0;
    ssize_t ptrPos    = 0;
    while (ch != EOF) {
        ch = fgetc(bfFile);
        switch (ch) {
        case '>':
            fputs("ptr++;", cFile);
            ptrPos++;
            break;
        case '<':
            fputs("ptr--;", cFile);
            ptrPos--;
            break;
        case '+': fputs("(*ptr)++;", cFile); break;
        case '-': fputs("(*ptr)--;", cFile); break;
        case '.': fputs("putchar(*ptr);", cFile); break;
        case ',': fputs("*ptr=getchar();", cFile); break;
        case '[':
            fputs("while(*ptr){", cFile);
            loopDepth++;
            break;
        case ']':
            fputs("}", cFile);
            loopDepth--;
            break;
        }
    }

    fputc('}', cFile);
    fclose(cFile);

    if (loopDepth != 0) {
        puts(
            "\033[0;31mError:\033[m Incomplete brackets found.\nCompilation "
            "terminated");
        remove(cFileName);
        return 1;
    } else if (ptrPos < 0 || ptrPos >= 30000) {
        puts(
            "\033[0;31mError:\033[m Data pointer found to be out of bounds.\n"
            "Compilation terminated");
        remove(cFileName);
        return 1;
    }

    pid_t runGCC = fork();
    if (runGCC == 0) {
        execl("/usr/bin/gcc", "gcc", "-o", outFileName, cFileName, NULL);
    }
    wait(NULL);
    remove(cFileName);
}
