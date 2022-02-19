/** @file
  Interfejs klasy stos wielomianów.

  @author Michał Napiórkowski
  @date 2021
*/

#ifndef STACK_H
#define STACK_H

#include <stdbool.h>
#include "poly.h"

/**
 * Początkowy rozmiar stosu.
 */
#define INITIAL_STACK_SIZE 16

/**
 * To jest struktura przechowująca stos wielomianów.
 */
typedef struct PolyStack {
    int top; ///< indeks szczytowego elementu
    size_t capacity; ///< pojemność stosu
    Poly *polys; ///< tablica wielomianów
} PolyStack;

/**
 * Tworzy pusty stos o zadanej pojemności.
 * Ustawia top na -1.
 * @param[in] capacity : pojemność stosu
 * @return stos
 */
PolyStack StackInit(size_t capacity);

/**
 * Zwiększa rozmiar pamięci zaalokowanej na przechowywanie stosu.
 * @param[in,out] stack : stos
 */
void StackResize(PolyStack *stack);

/**
 * Usuwa stos z pamięci.
 * @param[in] stack : usuwany stos
 */
void StackClear(PolyStack *stack);

/**
 * Czy stos jest pełny?
 * @param[in] stack : stos
 * @return czy jest pełny
 */
bool StackIsFull(PolyStack *stack);

/**
 * Czy stos jest pusty?
 * @param[in] stack : stos
 * @return czy jest pusty
 */
bool StackIsEmpty(PolyStack *stack);

/**
 * Wstawia wielomian na stos.
 * Jeśli stos jest pełny, zwiększa jego rozmiar.
 * @param[in,out] stack : stos
 * @param[in] p : wielomian
 */
void StackPush(PolyStack *stack, Poly p);

/**
 * Zdejmuje element ze szczytu stosu.
 * @param[in,out] stack : stos
 */
void StackPop(PolyStack *stack);

/**
 * Zwraca element znajdujący się na szczycie stosu
 * @param[in] stack : stos
 * @param[out] empty : czy stos jest pusty
 * @return wielomian ze szczytu
 */
Poly StackTop(PolyStack *stack, bool *empty);

#endif //STACK_H
