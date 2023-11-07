/**
 * @file kyber768.c
 * @brief Kyber-768 KEM
 *
 * @section License
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2010-2023 Oryx Embedded SARL. All rights reserved.
 *
 * This file is part of CycloneCRYPTO Open.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 2.3.0
 **/

//Switch to the appropriate trace level
#define TRACE_LEVEL CRYPTO_TRACE_LEVEL

//Dependencies
#include "core/crypto.h"
#include "pqc/kyber768.h"

//Check crypto library configuration
#if (KYBER768_SUPPORT == ENABLED)

//Dependencies (liboqs)
#include <oqs/oqs.h>

//Common interface for key encapsulation mechanisms (KEM)
const KemAlgo kyber768KemAlgo =
{
   "Kyber-768",
   KYBER768_PUBLIC_KEY_LEN,
   KYBER768_SECRET_KEY_LEN,
   KYBER768_CIPHERTEXT_LEN,
   KYBER768_SHARED_SECRET_LEN,
   (KemAlgoGenerateKeyPair) kyber768GenerateKeyPair,
   (KemAlgoEncapsulate) kyber768Encapsulate,
   (KemAlgoDecapsulate) kyber768Decapsulate
};


/**
 * @brief Key pair generation
 * @param[in] prngAlgo PRNG algorithm
 * @param[in] prngContext Pointer to the PRNG context
 * @param[out] pk Public key
 * @param[out] sk Secret key
 * @return Error code
 **/

error_t kyber768GenerateKeyPair(const PrngAlgo *prngAlgo, void *prngContext,
   uint8_t *pk, uint8_t *sk)
{
   OQS_STATUS status;

   //Key pair generation
   status = OQS_KEM_kyber_768_keypair(pk, sk);

   //Return status code
   return (status == OQS_SUCCESS) ? NO_ERROR : ERROR_FAILURE;
}


/**
 * @brief Encapsulation algorithm
 * @param[in] prngAlgo PRNG algorithm
 * @param[in] prngContext Pointer to the PRNG context
 * @param[out] ct Ciphertext
 * @param[out] ss Shared secret
 * @param[in] pk Public key
 * @return Error code
 **/

error_t kyber768Encapsulate(const PrngAlgo *prngAlgo, void *prngContext,
   uint8_t *ct, uint8_t *ss, const uint8_t *pk)
{
   OQS_STATUS status;

   //Encapsulation algorithm
   status = OQS_KEM_kyber_768_encaps(ct, ss, pk);

   //Return status code
   return (status == OQS_SUCCESS) ? NO_ERROR : ERROR_FAILURE;
}


/**
 * @brief Decapsulation algorithm
 * @param[out] ss Shared secret
 * @param[in] ct Ciphertext
 * @param[in] sk Secret key
 * @return Error code
 **/

error_t kyber768Decapsulate(uint8_t *ss, const uint8_t *ct, const uint8_t *sk)
{
   OQS_STATUS status;

   //Decapsulation algorithm
   status = OQS_KEM_kyber_768_decaps(ss, ct, sk);

   //Return status code
   return (status == OQS_SUCCESS) ? NO_ERROR : ERROR_FAILURE;
}

#endif
