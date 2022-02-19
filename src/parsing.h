/** @file
  Funkcje parsujące wielomiany i komendy kalkulatora
  oraz ich funkcje pomocnicze.

  @author Michał Napiórkowski
  @date 2021
*/

#ifndef PARSING_H
#define PARSING_H

#include "input_output.h"
#include "stack.h"

/**
 * Sprawdza, czy napis reprezentuje współczynnik.
 * @param[in] str : napis
 * @param[in] begin : indeks, od którego znaku sprawdzamy napis (włącznie)
 * @param[in] end : indeks, do którego znaku sprawdzamy napis (wyłącznie)
 * @return Czy napis jest współczynnikiem?
 */
bool StringIsCoeff(StringWithSize str, int begin, int end);

/**
 * Parsuje napis reprezentujący współczynnik do tego współczynnika.
 * @param[in] str : napis
 * @param[in] begin : indeks znaku od którego parsujemy
 * @param[out] in_range : czy podczas parsowania został przekroczony zakres typu
 * @return współczynnik
 */
poly_coeff_t CoeffFromString(StringWithSize str, int begin, bool *in_range);

/**
 * Parsuje napis reprezentujący wielomian do tego wielomianu.
 * @param[in] str : napis
 * @param[in] begin : indeks początkowy
 * @param[in] end : indeks końcowy
 * @param[out] correct : czy wielomian jest poprawny
 * @param[out] in_range : czy liczby mieszczą się w zakresach
 * @return wielomian
 */
Poly PolyFromString(StringWithSize str, int begin, int end,
                    bool *correct, bool *in_range);

/**
 * Sprawdza, czy napis jest poprawną komendą bezargumentową
 * i podejmuje odpowiednie akcje dotyczące stosu.
 * @param[in] word : napis
 * @param[in] line : aktualny nr wiersza
 * @param[in,out] stack : stos
 */
void WordIsCommand(char word[], int line, PolyStack *stack);

/**
 * Sprawdza, czy napis jest poprawną komendą jednoargumentową
 * i podejmuje odpowiednie akcje dotyczące stosu.
 * @param[in] str : napis
 * @param[in,out] l : indeks następujący zaraz po komendzie
 * @param[in] line : aktualny nr wiersza
 * @param[in,out] stack : stos
 */
void WordIsCommandWithArg(StringWithSize str, int *l, int line, PolyStack *stack);

/**
 * Jeśli napis reprezentuje wielomian, zostaje wrzucony na stos.
 * @param[in] str : napis
 * @param[in] line : aktualny nr wiersza
 * @param[in,out] stack : stos
 */
void WordIsPoly(StringWithSize str, int line, PolyStack *stack);

/**
 * Analizuje wiersz pod kątem bycia wielomianem, komendą lub niepoprawnym.
 * @param[in] str : wiersz
 * @param[in] line : aktualny nr wiersza
 * @param[in,out] stack : stos
 */
void AnalyzeLine(StringWithSize str, int line, PolyStack *stack);

#endif //PARSING_H
