/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "ch.h"
#include "hal.h"
#include "api.h"
#include "rt_test_root.h"
#include "oslib_test_root.h"
#include <chprintf.h>
#include <stdlib.h>
#include <string.h>

/* Define the size of the message to sign */
#define MESSAGE_SIZE_BYTES 16
#define FALCON_SIG_WORKBUF_SIZE 6000

/* Define the Falcon signature parameters */
#define FALCON_PARAM_SET CRYPTO_SECRETKEYBYTES

/*Define the concurrent thread parameters*/
#define THREAD2_WORKBUF_SIZE 8192


/* Create the thread working areas */
static THD_WORKING_AREA(waThread1, FALCON_SIG_WORKBUF_SIZE);

static THD_WORKING_AREA(waThread2, THREAD2_WORKBUF_SIZE);

/* Define the thread function */
static THD_FUNCTION(Thread1, arg) {
    (void)arg;
    chRegSetThreadName("falcon_sign");

    /* Initialize the parameters for the signature */
    uint8_t message[MESSAGE_SIZE_BYTES];
    unsigned long long mlen = MESSAGE_SIZE_BYTES;
    unsigned long long smlen = CRYPTO_BYTES;
    uint8_t signature[CRYPTO_BYTES];
    uint8_t public_key[CRYPTO_PUBLICKEYBYTES];
    uint8_t secret_key[CRYPTO_SECRETKEYBYTES];

    /* Initialize the parameters for cycle count and timer */
    uint32_t cycle_counter;
    uint32_t start_cycle;
    uint32_t end_cycle;
    systime_t start_time;
    systime_t end_time;
    systime_t timer;
    uint32_t min = -1;
    uint32_t max = 0;
    uint32_t avg;
    uint8_t loops = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;


    /* Generate key pair */
    crypto_sign_keypair(public_key, secret_key);

    while (true) {
        DWT->CYCCNT = 0;
        loops ++;
        /* Generate a random message */
        for (int i = 0; i < MESSAGE_SIZE_BYTES; i++) {
            message[i] = rand() % 256;
        }

        chnWrite(&SD1, message, MESSAGE_SIZE_BYTES);

        /* Sign the message */
        start_time = chVTGetSystemTimeX();
        start_cycle = DWT->CYCCNT;
        crypto_sign(signature, &smlen, message, MESSAGE_SIZE_BYTES, secret_key);
        end_cycle = DWT->CYCCNT;
        end_time = chVTGetSystemTimeX();
        cycle_counter = end_cycle - start_cycle;
        timer = end_time - start_time;
        if (timer == 0) break;
        if (cycle_counter <= min) min = cycle_counter;
        if (cycle_counter >= max) max = cycle_counter;
        avg = avg / loops * (loops - 1) + cycle_counter / loops;
        if (avg==0) break;

        /* Verify the signature (for testing purposes) */
        int signature_verified = crypto_sign_open(message, &mlen, signature, smlen, public_key);

        /* Do something with the signature (e.g. send it over a network) */
        if (signature_verified==0){
        chnWrite(&SD1, signature, 16);
        }
        else break;

        /* Wait for some time before signing the next message */
        palSetPad(GPIOD, GPIOD_LED3);       // Orange.
        chThdSleepMilliseconds(1000);
        palClearPad(GPIOD, GPIOD_LED3);     // Orange.
        chThdSleepMilliseconds(500);

    }
}

/* Define the concurrent thread function */

static THD_FUNCTION(Thread2, arg) {
  (void)arg;

  while (true) {
    palSetPad(GPIOD, GPIOD_LED4);
    chThdSleepMilliseconds(500);
    palClearPad(GPIOD, GPIOD_LED4);
    chThdSleepMilliseconds(500);
  }
}

/*
 * Application entry point.
 */

int main(void) {

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */

  /* Initialize ChibiOS */
  halInit();
  chSysInit();

  sdStart(&SD1, NULL);
  sdStart(&SD2, NULL);
  palSetPadMode(GPIOA, 2, PAL_MODE_ALTERNATE(7));
  palSetPadMode(GPIOA, 3, PAL_MODE_ALTERNATE(7));


  /*
   * Creates the example thread.
   */

  chThdCreateStatic(waThread1, sizeof(waThread1), LOWPRIO, Thread1, NULL);
//  chThdCreateStatic(waThread2, sizeof(waThread2), HIGHPRIO, Thread2, NULL);

  /*
   * Normal main() thread activity, in this demo it does nothing except
   * sleeping in a loop and check the button state.
   */

  /* Infinite loop */
  while (true) {
    if (palReadPad(GPIOA, GPIOA_BUTTON)) {
      test_execute((BaseSequentialStream *)&SD2, &rt_test_suite);
      test_execute((BaseSequentialStream *)&SD2, &oslib_test_suite);
    }
    chThdSleepMilliseconds(1000);
  }
}

