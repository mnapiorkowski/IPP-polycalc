/** @file
  Implementacja funkcji do alokowania i realokowania pamięci.

  @author Michał Napiórkowski
  @date 2021
*/

#include <stdlib.h>
#include "mallocs.h"
#include "input_output.h"
#include "stack.h"

size_t MultiplySize(size_t x) {
    return 1 + RESIZE_FACTOR * x;
}

void SafeMonoMalloc(Mono *monos[], size_t size) {
    *monos = malloc(size * sizeof(Mono));
    if (*monos == NULL) {
        exit(1);
    }
}

void SafeMonoRealloc(Mono *monos[], size_t size) {
    *monos = realloc(*monos, size * sizeof(Mono));
    if (*monos == NULL) {
        exit(1);
    }
}

void SafeStackMalloc(PolyStack *stack) {
    stack->polys = malloc(stack->capacity * sizeof(Poly));
    if (stack->polys == NULL) {
        exit(1);
    }
}

void SafeStackRealloc(PolyStack *stack) {
    stack->capacity = MultiplySize(stack->capacity);
    stack->polys = realloc(stack->polys, stack->capacity * sizeof(Poly));
    if (stack->polys == NULL) {
        exit(1);
    }
}

void ReallocStringIfNecessary(StringWithSize *str) {
    if (str->length == (int)str->size) {
        str->size = MultiplySize(str->size);
        str->A = realloc(str->A, (str->size) * sizeof(*(str->A)));
        if (str->A == NULL) {
            exit(1);
        }
    }
}