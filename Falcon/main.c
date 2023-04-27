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
#define FALCON_SIG_WORKBUF_SIZE 8192

/* Define the Falcon signature parameters */
#define FALCON_PARAM_SET CRYPTO_SECRETKEYBYTES

/*Define the concurrent thread parameters*/
#define MONOPOLIZE_WORKBUF_SIZE 256
#define MONOPOLIZE_TIME_MS 10
#define AVAILABLE_TIME_MS 20

/* Create a mutex */
static MUTEX_DECL(mutex);

/* Create the thread working areas */
static THD_WORKING_AREA(waThread1, FALCON_SIG_WORKBUF_SIZE);

static THD_WORKING_AREA(monopolize_wa, MONOPOLIZE_WORKBUF_SIZE);

/* Define the concurrent thread function */
static thread_t *monopolize;
static THD_FUNCTION(Monopolize, arg) {
  (void)arg;
  uint32_t start;
  uint32_t elapsed;
  uint32_t end;
  uint16_t loop;

  while (!chThdShouldTerminateX()) {
    start = chVTGetSystemTimeX();
    palSetPad(GPIOD, GPIOD_LED4);
    chMtxLock(&mutex);
//    chprintf((BaseSequentialStream *)&SD2, "Time elapsed %lu - loops: %lu \r\n", start - end, loop);
    chMtxUnlock(&mutex);
    elapsed = 0;
    loop = 0;
    while ((elapsed < MS2RTC(CH_CFG_ST_FREQUENCY, MONOPOLIZE_TIME_MS)) & (!chThdShouldTerminateX())){
      elapsed = chVTGetSystemTimeX() - start;
      loop ++;
    }
    palClearPad(GPIOD, GPIOD_LED4);
    if (chThdShouldTerminateX()) chThdExit(MSG_OK);
    end = chVTGetSystemTimeX();
    chThdSleepMilliseconds(AVAILABLE_TIME_MS);
  }
}

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
    uint32_t min = -1;
    uint32_t max = 0;
    float avg;
    systime_t start_time;
    systime_t end_time;
    systime_t timer;
    float avg_t;
    systime_t min_t = -1;
    systime_t max_t = 0;
    uint8_t loops = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;


    /* Generate key pair */
    crypto_sign_keypair(public_key, secret_key);

    while (loops<100) {
        DWT->CYCCNT = 0;
        loops ++;
        /* Generate a random message */
        for (int i = 0; i < MESSAGE_SIZE_BYTES; i++) {
            message[i] = rand() % 256;
        }

        /* Sign the message */
        palSetPad(GPIOD, GPIOD_LED3);       // Orange.
        monopolize = chThdCreateStatic(monopolize_wa, sizeof(monopolize_wa), HIGHPRIO, Monopolize, NULL);
        chSysLock();
        start_time = chVTGetSystemTimeX();
        start_cycle = DWT->CYCCNT;
        chSysUnlock();
        crypto_sign(signature, &smlen, message, MESSAGE_SIZE_BYTES, secret_key);
        chSysLock();
        end_cycle = DWT->CYCCNT;
        end_time = chVTGetSystemTimeX();
        chSysUnlock();
        chThdTerminate(monopolize);
        chThdWait(monopolize);
        palClearPad(GPIOD, GPIOD_LED3);     // Orange.
        cycle_counter = end_cycle - start_cycle;
        timer = end_time - start_time;
        if (timer <= min_t) min_t = timer;
        if (timer >= max_t) max_t = timer;
        avg_t = (avg_t / (float) loops * (float) (loops - 1) + (float) timer / (float) loops);
        if (cycle_counter <= min) min = cycle_counter;
        if (cycle_counter >= max) max = cycle_counter;
        avg = (avg / (float) loops * (float) (loops - 1) + (float) cycle_counter / (float) loops);
        if (avg==0) break;

        chMtxLock(&mutex);
        chprintf((BaseSequentialStream *)&SD2,"Loop: %lu - Crypto_sign: Delta cycle count: %lu - Delta time: %lu\r\n", loops, cycle_counter, timer);
        chMtxUnlock(&mutex);

        /* Verify the signature (for testing purposes) */
        int signature_verified = crypto_sign_open(message, &mlen, signature, smlen, public_key);

        /* Do something with the signature (e.g. send it over a network) */
        if (signature_verified != 0) {
          chMtxLock(&mutex);
          chprintf((BaseSequentialStream *)&SD2, "Error - Signature non verified at loop %lu \r\n", loops);
          chMtxUnlock(&mutex);
          chThdExit(MSG_RESET);
        }

    }
    chMtxLock(&mutex);
    chprintf((BaseSequentialStream *)&SD2, "Test finished - Final Results \r\n\r\n - Cycles - \r\nAverage: %lu \r\nMinimum: %lu \r\nMaximum %lu \r\n\r\n - Time - \r\nAverage: %lu ms \r\nMinimum: %lu ms \r\nMaximum: %lu ms", (uint32_t) avg, min, max, (systime_t) avg_t, min_t, max_t);
    chMtxUnlock(&mutex);
    chThdExit(MSG_OK);
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

