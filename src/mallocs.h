/** @file
  Funkcje do alokowania i realokowania pamięci.

  @author Michał Napiórkowski
  @date 2021
*/

#ifndef MALLOCS_H
#define MALLOCS_H

#include "poly.h"
#include "input_output.h"
#include "stack.h"

/**
 * Ile razy zwiększamy pamięć przy realokacji.
 */
#define RESIZE_FACTOR 2

/**
 * Zwraca liczbę RESIZE_FACTOR razy większą
 * @param x : liczba (rozmiar)
 * @return większa liczba
 */
size_t MultiplySize(size_t x);

/**
 * Alokuje pamięć na tablicę jednomianów.
 * W przypadku błędu funkcji malloc, kończy wykonywanie programu z kodem 1.
 * @param[in] monos : tablica jednomianów
 * @param[in] size : na ile elementów chcemy zaalokować pamięć
 */
void SafeMonoMalloc(Mono *monos[], size_t size);

/**
 * Zmienia rozmiar pamięci przeznaczonej na tablicę jednomianów.
 * W przypadku błędu funkcji realloc, kończy wykonywanie programu z kodem 1.
 * @param[in] monos : tablica jednomianów
 * @param[in] size : na ile elementów chcemy realokować pamięć
 */
void SafeMonoRealloc(Mono *monos[], size_t size);

/**
 * Alokuje pamięć na stos wielomianów.
 * W przypadku błędu funkcji malloc, kończy wykonywanie programu z kodem 1.
 * @param[in] stack : stos
 */
void SafeStackMalloc(PolyStack *stack);

/**
 * Zmienia rozmiar pamięci przeznaczonej na stos wielomianów.
 * W przypadku błędu funkcji realloc, kończy wykonywanie programu z kodem 1.
 * @param[in] stack : stos
 */
void SafeStackRealloc(PolyStack *stack);

/**
 * Zmienia rozmiar pamięci przeznaczonej na napis.
 * W przypadku błędu funkcji realloc, kończy wykonywanie programu z kodem 1.
 * @param str : napis
 */
void ReallocStringIfNecessary(StringWithSize *str);

#endif //MALLOCS_H
