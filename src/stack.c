/** @file
  Implementacja stosu wielomianów.

  @author Michał Napiórkowski
  @date 2021
*/

#include <stdlib.h>
#include <stdbool.h>
#include "stack.h"
#include "mallocs.h"

PolyStack StackInit(size_t capacity) {
    PolyStack stack;
    stack.capacity = capacity;
    stack.top = -1;
    SafeStackMalloc(&stack);
    return stack;
}

void StackResize(PolyStack *stack) {
    SafeStackRealloc(stack);
}

void StackClear(PolyStack *stack) {
    for (int i = 0; i <= stack->top; i++) {
        Poly p = stack->polys[i];
        PolyDestroy(&p);
    }
    free(stack->polys);
}

bool StackIsFull(PolyStack *stack) {
    return stack->top == (int)(stack->capacity - 1);
}

bool StackIsEmpty(PolyStack *stack) {
    return stack->top == -1;
}

void StackPush(PolyStack *stack, Poly p) {
    if (StackIsFull(stack)) {
        StackResize(stack);
    }
    (stack->top)++;
    stack->polys[stack->top] = p;
}

void StackPop(PolyStack *stack) {
    if (!StackIsEmpty(stack)) {
        (stack->top)--;
    }
}

Poly StackTop(PolyStack *stack, bool *empty) {
    if (StackIsEmpty(stack)) {
        *empty = true;
        return PolyZero();
    }
    *empty = false;
    return stack->polys[stack->top];
}


