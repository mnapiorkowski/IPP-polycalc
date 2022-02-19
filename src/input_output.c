/** @file
  Implementacja funkcji do obsługi wejścia / wyjścia.

  @author Michał Napiórkowski
  @date 2021
*/

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "input_output.h"
#include "mallocs.h"

StringWithSize StringInit() {
    StringWithSize str;
    str.size = 0;
    str.length = 0;
    str.A = NULL;
    return str;
}

void ReadLine(StringWithSize *str, char first, bool *was_eof) {
    char c = '\0';
    str->length = 0;

    ReallocStringIfNecessary(str);
    str->A[0] = first;
    (str->length)++;
    if (first == '#') {
        str->A[0] = '\n';
        do {
            c = (char) getchar();
        } while (c != '\n' && c != EOF);
    } else if (first != '\n') {
        do {
            c = (char) getchar();
            ReallocStringIfNecessary(str);
            str->A[str->length] = c;
            (str->length)++;
        } while (c != '\n' && c != EOF);
    }
    if (c == EOF) {
        str->A[str->length - 1] = '\n';
        *was_eof = true;
    }
}

void PrintError(int line_number, char description[]) {
    if (fprintf(stderr, "ERROR %d %s\n", line_number, description) < 0)
        exit(1);
}

void PrintInt(int x) {
    if (printf("%d\n", x) < 0)
        exit(1);
}
