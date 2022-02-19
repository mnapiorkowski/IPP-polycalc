/** @file
  Funkcje do obsługi wejścia / wyjścia.

  @author Michał Napiórkowski
  @date 2021
*/

#ifndef INPUT_OUTPUT_H
#define INPUT_OUTPUT_H

#include <stdbool.h>

/**
 * To jest struktura przechowująca napis wraz z jego długością
 * oraz rozmiarem zaaloowanej nań pamięci.
 */
typedef struct StringWithSize {
    char *A; ///< napis
    int length; ///< długość napisu
    size_t size; ///< na ile elementów została zaalokowana pamięć
} StringWithSize;

/**
 * Ustawia początkowe wartości pól pustego napisu.
 * @return pusty napis
 */
StringWithSize StringInit();

/**
 * Wczytuje wiersz ze standardowego wejścia.
 * @param[out] str : wczytany wiersz
 * @param[in] first : pierwszy znak wiersza, wczytany wcześniej
 * @param[out] was_eof : czy wiersz zakończył się EOF-em
 */
void ReadLine(StringWithSize *str, char first, bool *was_eof);

/**
 * Wypisuje błąd na stderr. W wypadku niepowodzenia, kończy program z kodem 1.
 * @param[in] line_number : nr wiersza, w którym wystąpił błąd
 * @param[in] description : opis błędu
 */
void PrintError(int line_number, char description[]);

/**
 * Wypisuje liczbę całkowitą. W wypadku niepowodzenia, kończy program z kodem 1.
 * @param[in] x : liczba do wypisania
 */
void PrintInt(int x);

#endif //INPUT_OUTPUT_H
