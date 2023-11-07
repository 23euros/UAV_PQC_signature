#include <stdio.h>
#include "mpi/mpi.h"

error_t isProbablePrime(const Mpi *n, uint_t k, const PrngAlgo *prngAlgo, void *prngContext);
