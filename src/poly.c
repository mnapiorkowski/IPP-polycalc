/** @file
  Implementacja interfejsu klasy wielomianów rzadkich wielu zmiennych
  oraz funkcje pomocnicze.

  @author Michał Napiórkowski
  @date 2021
*/

#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "poly.h"
#include "mallocs.h"

void PolyPrint(const Poly *p) {
    if (PolyIsCoeff(p)) {
        if (printf("%ld", p->coeff) < 0)
            exit(1);
    } else {
        if (printf("(") < 0)
            exit(1);
        for (size_t i = 0; i < p->size; i++) {
            if (i)
                if (printf(")+(") < 0)
                    exit(1);
            PolyPrint(&p->arr[i].p);
            if (printf(",%d", MonoGetExp(p->arr + i)) < 0)
                exit(1);
        }
        if (printf(")") < 0)
            exit(1);
    }
}

void PolyDestroy(Poly *p) {
    assert(p);
    if (!PolyIsCoeff(p)) {
        for (size_t i = 0; i < p->size; i++) {
            MonoDestroy(&p->arr[i]);
        }
        free(p->arr);
    }
    *p = PolyZero();
}

void MonosArrayDestroy(size_t size, Mono *monos) {
    for (size_t j = 0; j < size; j++) {
        MonoDestroy(&monos[j]);
    }
    free(monos);
}

Poly PolyClone(const Poly *p) {
    assert(p);
    Poly copy = PolyZero();

    if (PolyIsCoeff(p)) {
        copy = PolyFromCoeff(p->coeff);
    } else {
        copy.size = p->size;
        SafeMonoMalloc(&copy.arr, copy.size);
        for (int i = 0; i < (int) copy.size; i++) {
            copy.arr[i] = MonoClone(&p->arr[i]);
        }
    }
    return copy;
}

/**
 * Porównuje wykładniki dwóch jednomianów.
 * @param[in] a : jednomian
 * @param[in] b : jednomian
 * @return 0, gdy wykładniki równe;
 * -1, gdy wykładnik a mniejszy od wykładnika b;
 * 1, w przeciwnym wypadku
 */
static int ExpComparator(const void *a, const void *b) {
    Mono _a = *(Mono *) a;
    Mono _b = *(Mono *) b;
    if (MonoGetExp(&_a) < MonoGetExp(&_b)) return -1;
    else if (MonoGetExp(&_a) == MonoGetExp(&_b)) return 0;
    else return 1;
}

/**
 * Sortuje tablicę jednomianów rosnąco po ich wykładnikach.
 * @param[in] size : liczba elementów tablicy
 * @param[in] monos[] : tablica jednomianów
 */
static void SortMonosByExp(size_t size, Mono monos[]) {
    qsort(monos, size, sizeof(Mono), ExpComparator);
}

Poly PolyAddToCoeff(const Poly *p, poly_coeff_t c) {
    Poly pp = PolyClone(p);
    Poly qq = PolyFromCoeff(c);
    SortMonosByExp(pp.size, pp.arr);

    if (c != 0) {
        if (MonoGetExp(&pp.arr[0]) == 0) {
            Poly tmp = PolyAdd(&pp.arr[0].p, &qq);

            if (!PolyIsZero(&tmp)) {
                PolyDestroy(&pp.arr[0].p);
                pp.arr[0].p = tmp;
            } else { // usuwamy jednomian przy zerowym wykładniku
                // nadaję mu najwyższy wykładnik, sortuję
                //  i realokuję na o 1 mniej elementów
                pp.arr[0].exp = MonoGetExp(&pp.arr[pp.size - 1]) + 1;
                SortMonosByExp(pp.size, pp.arr);
                pp.size--;
                SafeMonoRealloc(&pp.arr, pp.size);
            }
        } else { // wielomian pp nie ma jednomianu przy wykładniku 0
            pp.size++;
            SafeMonoRealloc(&pp.arr, pp.size);
            pp.arr[pp.size - 1] = MonoFromPoly(&qq, 0);
            SortMonosByExp(pp.size, pp.arr);
        }
    }
    return pp;
}

void PolyToCoeff(Poly *p) {
    poly_coeff_t co = p->arr[0].p.coeff;
    PolyDestroy(p);
    p->coeff = co;
}

/**
 * Wpisuje kopię jednomianu wielomianu pp do jednomianu wielomianu sum.
 * Funkcja pomocnicza dla funkcji FillPolySum.
 * @param[in] pp : wielomian, z którego kopiujemy
 * @param[in,out] i : indeks jednomianu, który kopiujemy
 * @param[in,out] sum : wielomian, do którego wpisujemy
 * @param[in,out] num : indeks jednomianu, do którego wpisujemy
 */
static void FillPolySumHelper(const Poly pp, size_t *i, Poly *sum, size_t *num) {
    if (!PolyIsZero(&pp.arr[*i].p)) {
        sum->arr[*num].exp = MonoGetExp(&pp.arr[*i]);
        sum->arr[*num].p = PolyClone(&pp.arr[*i].p);
        (*num)++;
    }
    (*i)++;
}

/**
 * Wypełnia wielomian, będący sumą dwóch wielomianów.
 * Funkcja pomocnicza dla funkcji PolyAdd.
 * @param[in,out] sum : wielomian zawierający sumę pp i qq
 * @param[in] pp : wielomian
 * @param[in] qq : wielomian
 */
static void FillPolySum(Poly *sum, const Poly pp, const Poly qq) {
    size_t i = 0, j = 0, num = 0;
    size_t ps = pp.size;
    size_t qs = qq.size;

    while (i < ps && j < qs) {
        if (MonoGetExp(&pp.arr[i]) == MonoGetExp(&qq.arr[j])) {
            Poly tmp = PolyAdd(&pp.arr[i].p, &qq.arr[j].p);

            if (!PolyIsZero(&tmp)) {
                sum->arr[num].exp = MonoGetExp(&pp.arr[i]);
                sum->arr[num].p = tmp;
                num++;
            }
            i++;
            j++;
        } else if (MonoGetExp(&pp.arr[i]) < MonoGetExp(&qq.arr[j])) {
            FillPolySumHelper(pp, &i, sum, &num);
        } else {
            FillPolySumHelper(qq, &j, sum, &num);
        }
    }

    // trzeba dokończyć jeśli została jakaś końcówka z p albo q
    while (i < ps) {
        FillPolySumHelper(pp, &i, sum, &num);
    }
    while (j < qs) {
        FillPolySumHelper(qq, &j, sum, &num);
    }

    sum->size = num;
}

Poly PolyAdd(const Poly *p, const Poly *q) {
    assert(p && q);
    Poly sum;

    if (PolyIsCoeff(p) && PolyIsCoeff(q)) {
        return PolyFromCoeff(p->coeff + q->coeff);
    }

    if (PolyIsCoeff(p)) {
        return PolyAddToCoeff(q, p->coeff);
    } else if (PolyIsCoeff(q)) {
        return PolyAddToCoeff(p, q->coeff);
    }

    Poly pp = PolyClone(p);
    Poly qq = PolyClone(q);
    SortMonosByExp(pp.size, pp.arr);
    SortMonosByExp(qq.size, qq.arr);

    SafeMonoMalloc(&sum.arr, pp.size + qq.size);

    FillPolySum(&sum, pp, qq);

    if (sum.size == 0) { // wszystkie jednomiany się wyzerowały ze sobą
        PolyDestroy(&sum);
    } else {
        SafeMonoRealloc(&sum.arr, sum.size);
    }

    if (sum.size == 1 && MonoGetExp(&sum.arr[0]) == 0 &&
        PolyIsCoeff(&sum.arr[0].p)) {
        // otrzymaliśmy wielomian tożsamościowo równy współczynnikowi
        PolyToCoeff(&sum);
    }

    PolyDestroy(&pp);
    PolyDestroy(&qq);
    return sum;
}

/**
 * Funkcja pomocnicza dla PolyAddMonos, PolyOwnMonos i PolyCloneMonos.
 * Przejmuje na własność zawartość tablicy @p monos i jej zawartość.
 * @param[in] count : liczba jednomianów
 * @param[in] monos : tablica jednomianów
 * @return wielomian będący sumą jednomianów
 */
static Poly PolyAddMonosHelper(size_t count, Mono monos[]) {
    Poly res;
    SortMonosByExp(count, monos);
    SafeMonoMalloc(&res.arr, count);

    size_t i = 0, num = 0;
    while (i < count) {
        res.arr[num].p = PolyZero();
        res.arr[num].exp = MonoGetExp(&monos[i]);

        size_t j = i;
        while (j < count && MonoGetExp(&monos[j]) == MonoGetExp(&monos[i])) {
            Poly tmp = PolyAdd(&res.arr[num].p, &monos[j].p);
            PolyDestroy(&res.arr[num].p);
            res.arr[num].p = tmp;
            j++;
        }
        if (!PolyIsZero(&res.arr[num].p)) {
            num++;
        }
        i = j;
    }
    res.size = num;
    if (res.size == 0) {
        PolyDestroy(&res);
    } else {
        SafeMonoRealloc(&res.arr, res.size);
    }

    if (res.size == 1 && MonoGetExp(&res.arr[0]) == 0 &&
        PolyIsCoeff(&res.arr[0].p)) {
        // otrzymaliśmy wielomian tożsamościowo równy współczynnikowi
        PolyToCoeff(&res);
    }
    MonosArrayDestroy(count, monos);
    return res;
}

Poly PolyAddMonos(size_t count, const Mono monos[]) {
    Mono *mcopy;
    SafeMonoMalloc(&mcopy, count);
    for (size_t i = 0; i < count; i++) {
        mcopy[i] = monos[i];
    }

    return PolyAddMonosHelper(count, mcopy);
    /*
    SortMonosByExp(count, mcopy);

    SafeMonoMalloc(&res.arr, count);

    size_t i = 0, num = 0;
    while (i < count) {
        res.arr[num].p = PolyZero();
        res.arr[num].exp = MonoGetExp(&mcopy[i]);

        size_t j = i;
        while (j < count && MonoGetExp(&mcopy[j]) == MonoGetExp(&mcopy[i])) {
            Poly tmp = PolyAdd(&res.arr[num].p, &mcopy[j].p);
            PolyDestroy(&res.arr[num].p);
            res.arr[num].p = tmp;
            j++;
        }
        if (!PolyIsZero(&res.arr[num].p)) {
            num++;
        }
        i = j;
    }
    res.size = num;
    if (res.size == 0) {
        PolyDestroy(&res);
    } else {
        SafeMonoRealloc(&res.arr, res.size);
    }

    if (res.size == 1 && MonoGetExp(&res.arr[0]) == 0 &&
        PolyIsCoeff(&res.arr[0].p)) {
        // otrzymaliśmy wielomian tożsamościowo równy współczynnikowi
        PolyToCoeff(&res);
    }
    MonosArrayDestroy(count, mcopy);
    return res;
     */
}

Poly PolyOwnMonos(size_t count, Mono *monos) {
    if (count == 0 || monos == NULL) {
        MonosArrayDestroy(count, monos);
        return PolyZero();
    }

    return PolyAddMonosHelper(count, monos);
    /*
    SafeMonoMalloc(&res.arr, count);
    SortMonosByExp(count, monos);

    size_t i = 0, num = 0;
    while (i < count) {
        res.arr[num].p = PolyZero();
        res.arr[num].exp = MonoGetExp(&monos[i]);

        size_t j = i;
        while (j < count && MonoGetExp(&monos[j]) == MonoGetExp(&monos[i])) {
            Poly tmp = PolyAdd(&res.arr[num].p, &monos[j].p);
            PolyDestroy(&res.arr[num].p);
            res.arr[num].p = tmp;
            j++;
        }
        if (!PolyIsZero(&res.arr[num].p)) {
            num++;
        }
        i = j;
    }
    res.size = num;
    if (res.size == 0) {
        PolyDestroy(&res);
    } else {
        SafeMonoRealloc(&res.arr, res.size);
    }

    if (res.size == 1 && MonoGetExp(&res.arr[0]) == 0 &&
        PolyIsCoeff(&res.arr[0].p)) {
        // otrzymaliśmy wielomian tożsamościowo równy współczynnikowi
        PolyToCoeff(&res);
    }
    MonosArrayDestroy(count, monos);
    return res;
     */
}

Poly PolyCloneMonos(size_t count, const Mono monos[]) {
    if (count == 0 || monos == NULL) {
        return PolyZero();
    }

    Mono *mclone;
    SafeMonoMalloc(&mclone, count);
    for (size_t i = 0; i < count; i++) {
        mclone[i] = MonoClone(&monos[i]);
    }

    return PolyAddMonosHelper(count, mclone);
    /*
    SortMonosByExp(count, mclone);

    SafeMonoMalloc(&res.arr, count);

    size_t i = 0, num = 0;
    while (i < count) {
        res.arr[num].p = PolyZero();
        res.arr[num].exp = MonoGetExp(&mclone[i]);

        size_t j = i;
        while (j < count && MonoGetExp(&mclone[j]) == MonoGetExp(&mclone[i])) {
            Poly tmp = PolyAdd(&res.arr[num].p, &mclone[j].p);
            PolyDestroy(&res.arr[num].p);
            res.arr[num].p = tmp;
            j++;
        }
        if (!PolyIsZero(&res.arr[num].p)) {
            num++;
        }
        i = j;
    }
    res.size = num;
    if (res.size == 0) {
        PolyDestroy(&res);
    } else {
        SafeMonoRealloc(&res.arr, res.size);
    }

    if (res.size == 1 && MonoGetExp(&res.arr[0]) == 0 &&
        PolyIsCoeff(&res.arr[0].p)) {
        // otrzymaliśmy wielomian tożsamościowo równy współczynnikowi
        PolyToCoeff(&res);
    }
    MonosArrayDestroy(count, mclone);
    return res;
     */
}

void CoeffToPoly(const Poly *p, Poly *pp) {
    pp->size = 1;
    SafeMonoMalloc(&pp->arr, 1);
    pp->arr[0] = MonoFromPoly(p, 0);
}

Poly PolyMul(const Poly *p, const Poly *q) {
    assert(p && q);

    if (PolyIsCoeff(p) && PolyIsCoeff(q)) {
        return PolyFromCoeff(p->coeff * q->coeff);
    }
    if (PolyIsZero(p) || PolyIsZero(q)) {
        return PolyZero();
    }

    Poly pp, qq;
    if (PolyIsCoeff(p)) {
        CoeffToPoly(p, &pp);
        qq = *q;
    } else if (PolyIsCoeff(q)) {
        CoeffToPoly(q, &qq);
        pp = *p;
    } else {
        pp = *p;
        qq = *q;
    }

    size_t ps = pp.size, qs = qq.size;
    Mono *monos;
    SafeMonoMalloc(&monos, ps * qs);

    for (size_t i = 0; i < ps; i++) {
        for (size_t j = 0; j < qs; j++) {
            Poly product = PolyMul(&pp.arr[i].p, &qq.arr[j].p);

            if (PolyIsZero(&product)) {
                monos[i * qs + j] = MonoFromPoly(&product, 0);
            } else {
                poly_exp_t new_exp = MonoGetExp(&pp.arr[i]) +
                                     MonoGetExp(&qq.arr[j]);
                monos[i * qs + j] = MonoFromPoly(&product, new_exp);
            }
        }
    }

    if (PolyIsCoeff(p) && !PolyIsZero(p)) {
        PolyDestroy(&pp);
    }
    if (PolyIsCoeff(q) && !PolyIsZero(q)) {
        PolyDestroy(&qq);
    }

    Poly res = PolyAddMonos(ps * qs, monos);
    free(monos);
    return res;
}

/**
 * Zmienia współczynniki wielomianu na przeciwne.
 * Funkcja pomocnicza dla funkcji PolyNeg.
 * @param[in] p : wielomian
 * @return : @f$-p@f$
 */
static Poly PolyNegHelper(Poly *p) {
    assert(p);
    if (PolyIsCoeff(p)) {
        *p = PolyFromCoeff(-p->coeff);
    } else {
        for (size_t i = 0; i < p->size; i++) {
            p->arr[i].p = PolyNegHelper(&p->arr[i].p);
        }
    }
    return *p;
}

Poly PolyNeg(const Poly *p) {
    Poly clone = PolyClone(p);
    return PolyNegHelper(&clone);
}

Poly PolySub(const Poly *p, const Poly *q) {
    Poly neg = PolyNeg(q);
    Poly sub = PolyAdd(p, &neg);
    PolyDestroy(&neg);
    return sub;
}

poly_exp_t PolyDegBy(const Poly *p, size_t var_idx) {
    assert(p);

    if (PolyIsZero(p)) {
        return -1;
    }
    if (PolyIsCoeff(p)) {
        return 0;
    }

    if (var_idx == 0) {
        SortMonosByExp(p->size, p->arr);
        return MonoGetExp(&p->arr[p->size - 1]);
    } else {
        poly_exp_t maxi = 0;
        for (size_t i = 0; i < p->size; i++) {
            poly_exp_t deg = PolyDegBy(&p->arr[i].p, var_idx - 1);
            if (deg > maxi) {
                maxi = deg;
            }
        }
        return maxi;
    }
}

poly_exp_t PolyDeg(const Poly *p) {
    assert(p);

    if (PolyIsZero(p)) {
        return -1;
    }
    if (PolyIsCoeff(p)) {
        return 0;
    }

    poly_exp_t maxi = 0;
    for (size_t i = 0; i < p->size; i++) {
        poly_exp_t deg = PolyDeg(&p->arr[i].p) + MonoGetExp(&p->arr[i]);
        if (deg > maxi) {
            maxi = deg;
        }
    }
    return maxi;
}

bool PolyIsEq(const Poly *p, const Poly *q) {
    assert(p && q);

    if (PolyIsCoeff(p) && PolyIsCoeff(q)) {
        return p->coeff == q->coeff;
    } else if (!PolyIsCoeff(p) && !PolyIsCoeff(q)) {
        if (p->size == q->size) {
            SortMonosByExp(p->size, p->arr);
            SortMonosByExp(q->size, q->arr);

            for (size_t i = 0; i < p->size; i++) {
                if (MonoGetExp(&p->arr[i]) != MonoGetExp(&q->arr[i]) ||
                    !PolyIsEq(&p->arr[i].p, &q->arr[i].p)) {
                    return false;
                }
            }
        } else {
            return false;
        }
    } else { // jeden jest coeffem, drugi nie
        return false;
    }
    return true;
}

/**
 * Szybkie potęgowanie.
 * @param[in] x : podstawa potęgi
 * @param[in] exp : wykładnik potęgi
 * @return : @f$x^{exp}@f$
 */
static poly_coeff_t CoeffPower(poly_coeff_t x, poly_exp_t exp) {
    assert(exp >= 0);

    poly_coeff_t res = 1;
    while (exp > 0) {
        if (exp % 2 == 1) {
            res *= x;
        }
        x *= x;
        exp /= 2;
    }
    return res;
}

Poly PolyAt(const Poly *p, poly_coeff_t x) {
    assert(p);

    if (PolyIsCoeff(p)) {
        return PolyFromCoeff(p->coeff);
    }
    Poly res = PolyZero();
    for (size_t i = 0; i < p->size; i++) {
        Poly coeff = PolyFromCoeff(CoeffPower(x, MonoGetExp(&p->arr[i])));
        Poly mul = PolyMul(&p->arr[i].p, &coeff);
        Poly tmp = PolyAdd(&res, &mul);
        PolyDestroy(&mul);
        PolyDestroy(&res);
        res = tmp;
    }
    return res;
}

Poly PolyPower(const Poly *p, poly_exp_t exp) {
    assert(exp >= 0);

    Poly clone = PolyClone(p);
    Poly res = PolyFromCoeff(1);
    while (exp > 0) {
        if (exp % 2 == 1) {
            Poly mul = PolyMul(&res, &clone);
            PolyDestroy(&res);
            res = mul;
        }
        Poly mul = PolyMul(&clone, &clone);
        PolyDestroy(&clone);
        clone = mul;
        exp /= 2;
    }
    PolyDestroy(&clone);
    return res;
}

Poly PolyCompose(const Poly *p, size_t k, const Poly q[]) {
    if (PolyIsCoeff(p)) {
        return PolyFromCoeff(p->coeff);
    }
    if (k == 0) {
        if (MonoGetExp(&(p->arr[0])) == 0)
            return PolyCompose(&(p->arr[0].p), k, NULL);
        else
            return PolyZero();
    }
    assert(k > 0 && q != NULL);

    Mono *monos = NULL;
    size_t count = 0;
    for (size_t i = 0; i < p->size; i++) {
        Poly comp = PolyCompose(&(p->arr[i].p), k - 1, q);
        Poly power = PolyPower(&q[k - 1], p->arr[i].exp);
        Poly mul = PolyMul(&comp, &power);
        PolyDestroy(&comp);
        PolyDestroy(&power);

        if (PolyIsCoeff(&mul)) {
            count++;
            SafeMonoRealloc(&monos, count);
            monos[count - 1] = MonoFromPoly(&mul, 0);
        } else {
            SafeMonoRealloc(&monos, count + mul.size);
            for (size_t j = 0; j < mul.size; j++) {
                count++;
                monos[count - 1] = mul.arr[j];
            }
            free(mul.arr);
        }
    }
    Poly res = PolyOwnMonos(count, monos);

    return res;
}

