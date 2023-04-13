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
#define FALCON_SIG_WORKBUF_SIZE 64500

/* Define the Falcon signature parameters */
#define FALCON_PARAM_SET CRYPTO_SECRETKEYBYTES


/*
 * This is a periodic thread that does absolutely nothing except flashing
 * a LED.
 */


static THD_WORKING_AREA(waThread1, FALCON_SIG_WORKBUF_SIZE);

/* Define the thread function */
static THD_FUNCTION(Thread1, arg) {
    (void)arg;
    chRegSetThreadName("falcon_sign");

    /* Create a buffer to hold the message to sign */
    uint8_t message[MESSAGE_SIZE_BYTES];
    uint8_t signature[CRYPTO_BYTES];
//    uint8_t public_key[CRYPTO_PUBLICKEYBYTES];
    uint8_t secret_key[CRYPTO_SECRETKEYBYTES];
    for (int i = 0; i < CRYPTO_SECRETKEYBYTES; i++) {
        secret_key[i] = rand() % 256;
    }
//    memset(secret_key, 0, sizeof(secret_key));
//    crypto_sign_keypair(public_key, secret_key);


    while (true) {
        /* Generate a random message */
        for (int i = 0; i < MESSAGE_SIZE_BYTES; i++) {
            message[i] = rand() % 256;
        }
//        memset(message, 0, sizeof(message));
        /* Sign the message */

        for (int i = 0; i < CRYPTO_BYTES; i++) {
            signature[i] = rand() % 256;
        }

        crypto_sign(signature, NULL, message, MESSAGE_SIZE_BYTES, secret_key);

        /* Verify the signature (for testing purposes) */
        chnWrite(&SD1, signature, 690);//        int signature_verified = crypto_sign_open(NULL, NULL, signature, CRYPTO_BYTES, public_key);*/

        /* Do something with the signature (e.g. send it over a network) */

        /* Wait for some time before signing the next message */
        palSetPad(GPIOD, GPIOD_LED3);       // Orange.
//        chprintf((BaseSequentialStream *)&SD1, "Hello, world!\r\n");
        chThdSleepMilliseconds(1000);
        palClearPad(GPIOD, GPIOD_LED3);     // Orange.
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

  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

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

