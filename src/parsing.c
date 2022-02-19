/** @file
  Implementacja funkcji parsujących wielomiany i komendy kalkulatora.

  @author Michał Napiórkowski
  @date 2021
*/

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include "stack.h"
#include "parsing.h"
#include "mallocs.h"
#include "input_output.h"

/**
 * Sprawdza, czy nie został przekroczony zakres danego typu
 * przy parsowaniu funkcjami strto*.
 * @return czy zakres został przekroczony?
 */
static bool IsInRange() {
    if (errno == ERANGE) {
        errno = 0;
        return false;
    }
    return true;
}

bool StringIsCoeff(StringWithSize str, int begin, int end) {
    int i = begin;
    if ((end - begin > 1 && str.A[i] == '-' &&
        (str.A[i + 1] >= '0' && str.A[i + 1] <= '9')) ||
        (str.A[i] >= '0' && str.A[i] <= '9')) {

        i++;
        while (i < end) {
            if (str.A[i] >= '0' && str.A[i] <= '9')
                i++;
            else
                return false;
        }
    } else {
        return false;
    }
    return true;
}

poly_coeff_t CoeffFromString(StringWithSize str, int begin, bool *in_range) {
    poly_coeff_t coeff = strtol(&str.A[begin], NULL, 10);
    *in_range = IsInRange();
    return coeff;
}

/**
 * Zlicza znaki '+' występujące w napisie reprezentującym wielomian.
 * Bierze pod uwagę tylko te, które poprzedza poprawne wyrażenie nawiasowe.
 * @param[in] str : napis reprezentujący wielomian
 * @param[in] begin : indeks początkowy (włącznie)
 * @param[in] end : indeks końcowy (wyłącznie)
 * @return liczba plusów
 */
static int CountPluses(StringWithSize str, int begin, int end) {
    int count = 0, open_br = 0, close_br = 0;
    for (int i = begin; i < end; i++) {
        if (str.A[i] == '+' && open_br == close_br) {
            count++;
        } else if (str.A[i] == '(') {
            open_br++;
        } else if (str.A[i] == ')') {
            close_br++;
        }
    }
    return count;
}

/**
 * Zlicza nawiasy otwierające i zamykające w napisie
 * oraz sprawdza indeks przecinka.
 * @param[in,out] i : aktualny indeks
 * @param[in] end : indeks, do którego sprawdzamy
 * @param[in] A : tablica znaków, którą sprawdzamy
 * @param[out] comma : indeks, pod którym występuje przecinek
 * @param[out] open_br : liczba nawiasów otwierających
 * @param[out] close_br : liczba nawiasów zamykających
 */
static void CountBrackets(int *i, int end, char A[],
                          int *comma, int *open_br, int *close_br) {
    while (*i < end && !(A[*i] == '+' && *open_br == *close_br)) {
        // przerywamy na '+' jeśli zgadzają się nawiasy
        if (A[*i] == ',') {
            *comma = *i;
        } else if (A[*i] == '(') {
            (*open_br)++;
        } else if (A[*i] == ')') {
            (*close_br)++;
        }
        (*i)++;
    }
}

Poly PolyFromString(StringWithSize str, int begin, int end,
                    bool *correct, bool *in_range) {
    Poly result;
    Mono *monos;
    int index = 0;
    int i = begin;
    int new_begin = begin + 1;
    int comma = -1;
    int open_br = 0;
    int close_br = 0;
    char *endptr;
    *correct = true;
    *in_range = true;

    if (str.A[begin] != '(' || str.A[end - 1] != ')') {
        *correct = false;
    }

    int pluses = CountPluses(str, begin, end);
    SafeMonoMalloc(&monos, (size_t)(pluses + 1));

    while (i < end) {
        CountBrackets(&i, end, str.A, &comma, &open_br, &close_br);

        if (comma < 0) {
            *correct = false;
        }
        // teraz i jest na '+' albo na '\n' kończącym stringa
        // albo na ',' jeśli jesteśmy wewnątrz rekurencji
        // comma to indeks pod którym jest ostatni przecinek w danym jednomianie
        if (comma + 1 == i - 1) { // jest coś takiego (xyz,)
            *correct = false;
        }

        for (int j = comma + 1; j < i - 1; j++) {
            if (str.A[j] < '0' || str.A[j] > '9') {
                *correct = false;
            }
        }

        if (*correct == false) {
            MonosArrayDestroy(index, monos);
            return PolyZero();
        }

        int exp = strtol(&str.A[comma + 1], &endptr, 10);
        if (!IsInRange() || exp > INT_MAX || exp < 0) {
            *in_range = false;
            MonosArrayDestroy(index, monos);
            return PolyZero();
        }

        Poly p;
        if (StringIsCoeff(str, new_begin, comma)) {
            poly_coeff_t coeff = CoeffFromString(str, new_begin, in_range);
            if (*in_range) {
                p = PolyFromCoeff(coeff);
            }
        } else {
            p = PolyFromString(str, new_begin, comma, correct, in_range);
        }

        if (*correct == false || *in_range == false) {
            MonosArrayDestroy(index, monos);
            return PolyZero();
        }

        Mono m = MonoFromPoly(&p, exp);
        monos[index] = m;
        index++;
        i++;
        new_begin = i + 1;
    }

    result = PolyAddMonos(index, monos);
    free(monos);
    return result;
}

/**
 * Wypisuje odpowiedni błąd jeśli stos jest pusty.
 * @param[in] empty : czy stos jest pusty
 * @param[in] line : aktualny nr wiersza
 * @return czy stos jest pusty?
 */
static bool TopIsEmpty(bool empty, int line) {
    if (empty) {
        PrintError(line, "STACK UNDERFLOW");
        return true;
    }
    return false;
}

void WordIsCommand(char word[], int line, PolyStack *stack) {
    bool empty;

    if (strcmp(word, "ZERO") == 0) {
        StackPush(stack, PolyZero());
    } else if (strcmp(word, "IS_COEFF") == 0) {
        Poly top = StackTop(stack, &empty);
        if (TopIsEmpty(empty, line))
            return;
        PrintInt((int) PolyIsCoeff(&top));
    } else if (strcmp(word, "IS_ZERO") == 0) {
        Poly top = StackTop(stack, &empty);
        if (TopIsEmpty(empty, line))
            return;
        PrintInt((int) PolyIsZero(&top));
    } else if (strcmp(word, "CLONE") == 0) {
        Poly top = StackTop(stack, &empty);
        if (TopIsEmpty(empty, line))
            return;
        Poly clone = PolyClone(&top);
        StackPush(stack, clone);
    } else if (strcmp(word, "ADD") == 0) {
        Poly p = StackTop(stack, &empty);
        if (TopIsEmpty(empty, line))
            return;
        StackPop(stack);
        Poly q = StackTop(stack, &empty);
        if (TopIsEmpty(empty, line)) {
            (stack->top)++;
            return;
        }
        StackPop(stack);
        Poly sum = PolyAdd(&p, &q);
        PolyDestroy(&p);
        PolyDestroy(&q);
        StackPush(stack, sum);
    } else if (strcmp(word, "MUL") == 0) {
        Poly p = StackTop(stack, &empty);
        if (TopIsEmpty(empty, line))
            return;
        StackPop(stack);
        Poly q = StackTop(stack, &empty);
        if (TopIsEmpty(empty, line)) {
            (stack->top)++;
            return;
        }
        StackPop(stack);
        Poly mul = PolyMul(&p, &q);
        PolyDestroy(&p);
        PolyDestroy(&q);
        StackPush(stack, mul);
    } else if (strcmp(word, "NEG") == 0) {
        Poly top = StackTop(stack, &empty);
        if (TopIsEmpty(empty, line))
            return;
        StackPop(stack);
        Poly neg = PolyNeg(&top);
        PolyDestroy(&top);
        StackPush(stack, neg);
    } else if (strcmp(word, "SUB") == 0) {
        Poly p = StackTop(stack, &empty);
        if (TopIsEmpty(empty, line))
            return;
        StackPop(stack);
        Poly q = StackTop(stack, &empty);
        if (TopIsEmpty(empty, line)) {
            (stack->top)++;
            return;
        }
        StackPop(stack);
        Poly sub = PolySub(&p, &q);
        PolyDestroy(&p);
        PolyDestroy(&q);
        StackPush(stack, sub);
    } else if (strcmp(word, "IS_EQ") == 0) {
        Poly p = StackTop(stack, &empty);
        if (TopIsEmpty(empty, line))
            return;
        StackPop(stack);
        Poly q = StackTop(stack, &empty);
        if (TopIsEmpty(empty, line)) {
            (stack->top)++;
            return;
        }
        StackPush(stack, p);
        PrintInt((int) PolyIsEq(&p, &q));
    } else if (strcmp(word, "DEG") == 0) {
        Poly top = StackTop(stack, &empty);
        if (TopIsEmpty(empty, line))
            return;
        PrintInt(PolyDeg(&top));
    } else if (strcmp(word, "PRINT") == 0) {
        Poly top = StackTop(stack, &empty);
        if (TopIsEmpty(empty, line))
            return;
        PolyPrint(&top);
        if (printf("\n") < 0)
            exit(1);
    } else if (strcmp(word, "POP") == 0) {
        Poly top = StackTop(stack, &empty);
        if (TopIsEmpty(empty, line))
            return;
        PolyDestroy(&top);
        StackPop(stack);
    } else {
        PrintError(line, "WRONG COMMAND");
    }
}

void WordIsCommandWithArg(StringWithSize str, int *l,
                          int line, PolyStack *stack) {
    bool empty;
    char *endptr;

    if (strcmp(str.A, "DEG_BY") == 0) {
        (*l)++;
        if (str.A[*l] >= '0' && str.A[*l] <= '9') {
            unsigned long index = strtoul(&str.A[*l], &endptr, 10);

            if (!IsInRange() || endptr != &str.A[str.length - 1]) {
                // błąd bo przekroczono zakres lub są jakieś dalsze znaki
                PrintError(line, "DEG BY WRONG VARIABLE");
            } else {
                Poly top = StackTop(stack, &empty);
                if (TopIsEmpty(empty, line))
                    return;
                PrintInt(PolyDegBy(&top, index));
            }
        } else {
            // błąd bo niedozwolony znak (np więcej niż jedna spacja)
            PrintError(line, "DEG BY WRONG VARIABLE");
        }
    } else if (strcmp(str.A, "AT") == 0) {
        (*l)++;
        if ((str.A[*l] >= '0' && str.A[*l] <= '9') || str.A[*l] == '-') {
            long x = strtol(&str.A[*l], &endptr, 10);

            if (!IsInRange() || endptr != &str.A[str.length - 1]) {
                // błąd bo są jakieś dalsze znaki
                PrintError(line, "AT WRONG VALUE");
            } else {
                Poly top = StackTop(stack, &empty);
                if (TopIsEmpty(empty, line))
                    return;
                Poly p = PolyAt(&top, x);
                PolyDestroy(&top);
                StackPop(stack);
                StackPush(stack, p);
            }
        } else {
            // błąd bo niedozwolony znak (np więcej niż jedna spacja)
            PrintError(line, "AT WRONG VALUE");
        }
    } else if (strcmp(str.A, "COMPOSE") == 0) {
        (*l)++;
        if (str.A[*l] >= '0' && str.A[*l] <= '9') {
            unsigned long long k = strtoull(&str.A[*l], &endptr, 10);
            if (!IsInRange() || endptr != &str.A[str.length - 1]) {
                // błąd bo przekroczono zakres lub są jakieś dalsze znaki
                PrintError(line, "COMPOSE WRONG PARAMETER");
            } else {
                if (k > stack->capacity) {
                    PrintError(line, "STACK UNDERFLOW");
                    return;
                }
                Poly p = StackTop(stack, &empty);
                if (TopIsEmpty(empty, line))
                    return;
                StackPop(stack);

                Poly *q = malloc(k * sizeof(Poly));
                if (q == NULL)
                    exit(1);

                for (unsigned long long j = 0; j < k; j++) {
                    q[j] = StackTop(stack, &empty);
                    if (TopIsEmpty(empty, line)) {
                        (stack->top) += j + 1;
                        free(q);
                        return;
                    }
                    StackPop(stack);
                }

                Poly composed = PolyCompose(&p, (size_t)k, q);
                PolyDestroy(&p);
                for (unsigned long long j = 0; j < k; j++) {
                    PolyDestroy(&q[j]);
                }
                free(q);
                StackPush(stack, composed);
            }
        } else {
            // błąd bo niedozwolony znak (np więcej niż jedna spacja)
            PrintError(line, "COMPOSE WRONG PARAMETER");
        }
    } else {
        // błąd bo nieprawidłowe polecenie
        PrintError(line, "WRONG COMMAND");
    }
}

void WordIsPoly(StringWithSize str, int line, PolyStack *stack) {
    Poly p;
    bool in_range;

    if (StringIsCoeff(str, 0, str.length - 1)) {
        poly_coeff_t coeff = CoeffFromString(str, 0, &in_range);
        if (!in_range) {
            PrintError(line, "WRONG POLY");
            return;
        }
        p = PolyFromCoeff(coeff);
        StackPush(stack, p);
    } else {
        bool correct;
        p = PolyFromString(str, 0, str.length - 1, &correct, &in_range);
        if (correct && in_range) {
            StackPush(stack, p);
        } else {
            PrintError(line, "WRONG POLY");
        }
    }
}

void AnalyzeLine(StringWithSize str, int line, PolyStack *stack) {
    assert(str.length >= 1);
    char first = str.A[0];

    if (first == '\n')
        return;

    if ((first >= 'A' && first <= 'Z') || (first >= 'a' && first <= 'z')) {
        int l = 1;
        while (str.A[l] != '\n' && str.A[l] != ' ' &&
               (str.A[l] < 9 || str.A[l] > 13)) {
            // dopóki nie natrafimy na znak biały
            if (str.A[l] == '\0') {
                PrintError(line, "WRONG COMMAND");
                return;
            }
            l++;
        }
        if (str.A[l] == ' ') {
            // zamieniamy spację po słowie na null char, żeby stringi działały
            str.A[l] = '\0';
        } else { // jest innym białym znakiem
            str.A[l] = '\0';
            if (strcmp(str.A, "AT") == 0) {
                PrintError(line, "AT WRONG VALUE");
                return;
            } else if (strcmp(str.A, "DEG_BY") == 0) {
                PrintError(line, "DEG BY WRONG VARIABLE");
                return;
            } else if (strcmp(str.A, "COMPOSE") == 0) {
                PrintError(line, "COMPOSE WRONG PARAMETER");
                return;
            }
        }

        if (l == str.length - 1) {
            WordIsCommand(str.A, line, stack);
        } else {
            WordIsCommandWithArg(str, &l, line, stack);
        }
    } else {
        WordIsPoly(str, line, stack);
    }
}