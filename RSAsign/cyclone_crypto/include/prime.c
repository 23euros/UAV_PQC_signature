#include <stdio.h>
#include "mpi/mpi.h"

// Function to perform Miller-Rabin primality test
error_t isProbablePrime(Mpi *n, uint_t k, const PrngAlgo *prngAlgo, void *prngContext) {
    if (n == NULL || n->data == NULL || n->size == 0) {
        return ERROR_INVALID_PARAMETER;
    }

    if (n->size == 1 && n->data[0] <= 1) {
        return ERROR_INVALID_VALUE; // n is not prime
    }


    // Initialize MPI for calculations
    Mpi r, d, x, a, n_minus_1;
    mpiInit(&r);
    mpiInit(&d);
    mpiInit(&x);
    mpiInit(&a);
    mpiInit(&n_minus_1);

    const int primes[] = {
        3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37,
        41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89,
        97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149,
        151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199,
        211, 223, 227, 229, 233, 239, 241, 251, 257, 263, 269,
        271, 277, 281, 283, 293, 307, 311, 313, 317, 331, 337,
        347, 349
    };


    for (uint_t i = 0; i < sizeof(primes) / sizeof(primes[0]); i++) {
      if (mpiDivInt(&d, &r, n, primes[i]) != 0){
        mpiFree(&r);
        mpiFree(&d);
        mpiFree(&x);
        mpiFree(&a);
        mpiFree(&n_minus_1);
        return ERROR_INVALID_VALUE;
      }
      if (mpiCompInt(&r, 0) == 0){
        mpiFree(&r);
        mpiFree(&d);
        mpiFree(&x);
        mpiFree(&a);
        mpiFree(&n_minus_1);
        return ERROR_INVALID_VALUE;
      }
    }

    // Set n_minus_1 to n - 1
    if (mpiSubInt(&n_minus_1, n, 1) != 0) {
        mpiFree(&r);
        mpiFree(&d);
        mpiFree(&x);
        mpiFree(&a);
        mpiFree(&n_minus_1);
        return ERROR_INVALID_VALUE;
    }

    int s = 0;
    if (mpiCopy(&d, &n_minus_1) != 0) {
        mpiFree(&r);
        mpiFree(&d);
        mpiFree(&x);
        mpiFree(&a);
        mpiFree(&n_minus_1);
        return ERROR_INVALID_VALUE;
    }

    while (mpiIsEven(&d)) {
        if (mpiShiftRight(&d, 1) != 0) {
            mpiFree(&r);
            mpiFree(&d);
            mpiFree(&x);
            mpiFree(&a);
            mpiFree(&n_minus_1);
            return ERROR_INVALID_VALUE;
        }
        s++;
    }

    int prime = 0;  // Initialize prime here

    for (uint_t i = 0; i < k; i++) {
        if (mpiRand(&a, (n->size * 32) -1 , prngAlgo, prngContext) != 0) {
            mpiFree(&r);
            mpiFree(&d);
            mpiFree(&x);
            mpiFree(&a);
            mpiFree(&n_minus_1);
            return ERROR_INVALID_VALUE;
        }
        if (mpiAddInt(&a, &a, 2) != 0) {
            mpiFree(&r);
            mpiFree(&d);
            mpiFree(&x);
            mpiFree(&a);
            mpiFree(&n_minus_1);
            return ERROR_INVALID_VALUE;
        }
        if (mpiExpMod(&x, &a, &d, n) != 0) {
            mpiFree(&r);
            mpiFree(&d);
            mpiFree(&x);
            mpiFree(&a);
            mpiFree(&n_minus_1);
            return ERROR_INVALID_VALUE;
        }

        if (mpiCompInt(&x, 1) == 0){
          mpiFree(&r);
          mpiFree(&d);
          mpiFree(&x);
          mpiFree(&a);
          mpiFree(&n_minus_1);
          return ERROR_INVALID_VALUE;
        }

        if (mpiComp(&x, &n_minus_1) == 0) {
            continue; // Passes this test, maybe prime
        }

        prime = 0;  // Reset prime

        for (int j = 0; j < s; j++) {
            if (mpiMulMod(&x, &x, &x, n) != 0) {
                mpiFree(&r);
                mpiFree(&d);
                mpiFree(&x);
                mpiFree(&a);
                mpiFree(&n_minus_1);
                return ERROR_INVALID_VALUE;
            }
            if (mpiCompInt(&x, 1) == 0) {
                return ERROR_INVALID_VALUE; // Definitely composite
            }
            if (mpiComp(&x, &n_minus_1) == 0) {
                prime = 1;
                break; // Passes this test, maybe prime
            }
        }

        if (!prime) {
          mpiFree(&r);
          mpiFree(&d);
          mpiFree(&x);
          mpiFree(&a);
          mpiFree(&n_minus_1);
          return ERROR_INVALID_VALUE; // Definitely composite
        }
    }

    // Free MPI resources
    mpiFree(&r);
    mpiFree(&d);
    mpiFree(&x);
    mpiFree(&a);
    mpiFree(&n_minus_1);

    return NO_ERROR; // Probably prime
}
