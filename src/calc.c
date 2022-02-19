/** @file
  Kalkulator operacji na wielomianach rzadkich wielu zmiennych.

  @author Michał Napiórkowski
  @date 2021
*/

#include <stdlib.h>
#include <stdio.h>
#include "stack.h"
#include "input_output.h"
#include "parsing.h"

/**
 * Funkcja main kalkulatora.
 * @return 0, jeśli program zakończył się poprawnie, 1 w.p.p.
 */
int main() {
    StringWithSize str = StringInit();
    PolyStack stack = StackInit(INITIAL_STACK_SIZE);
    char c;
    int line = 1;
    bool was_eof = false;

    while (!was_eof && (c = (char) getchar()) != EOF) {
        ReadLine(&str, c, &was_eof);
        AnalyzeLine(str, line, &stack);
        line++;
    }

    free(str.A);
    StackClear(&stack);
    return 0;
}
