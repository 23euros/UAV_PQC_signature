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
#include "rt_test_root.h"
#include "oslib_test_root.h"
#include "p256-cortex-m4.h"
#include "randombytes.h"
#include <chprintf.h>
#include <stdlib.h>
#include <string.h>

/* Define the size of the message to sign */
#define MESSAGE_SIZE_BYTES 32
#define SIG_WORKBUF_SIZE 8196

/*Define the concurrent thread parameters*/
#define MONOPOLIZE_WORKBUF_SIZE 1024
#define MONOPOLIZE_TIME_MS 10
#define SIGN // Uncomment this line to enable the signature evaluation code
//#define VERIFY // Uncomment this line to enable the verification evaluation code
static uint8_t available_ms_table[] = {200, 100, 70, 50, 40, 30, 25, 20, 15, 10, 8, 6, 5, 4, 3, 2, 1};
static uint8_t available_ms;

/* Create a mutex and the interruptions counter */
static MUTEX_DECL(mutex);
static int count = 0;

/* Create the thread working areas */
static THD_WORKING_AREA(waThread1, SIG_WORKBUF_SIZE);

static THD_WORKING_AREA(monopolize_wa, MONOPOLIZE_WORKBUF_SIZE);

/* Define the concurrent thread function */
static thread_t *monopolize;
static THD_FUNCTION(Monopolize, arg) {
  (void)arg;
  uint32_t start;
  uint32_t elapsed;
//  uint32_t end;
  count = 0;

  while (!chThdShouldTerminateX()) {
    count ++;
    start = chVTGetSystemTimeX();
    palSetPad(GPIOD, GPIOD_LED4);
//    chMtxLock(&mutex);
//    chprintf((BaseSequentialStream *)&SD2, "Time elapsed %lu - loops: %lu \r\n", start - end, loop);
//    chMtxUnlock(&mutex);
    elapsed = 0;
//    uint32_t matrix[10][10];
//    for (int i = 0; i < 10; i++) {
//            for (int j = 0; j < 10; j++) {
//                matrix[i][j] = rand();
//            }
//    }
//    chprintf((BaseSequentialStream *)&SD1,"%lu",matrix[rand() % 10][rand() % 10]);


    while ((elapsed < MS2RTC(CH_CFG_ST_FREQUENCY, MONOPOLIZE_TIME_MS)) & (!chThdShouldTerminateX())){
      elapsed = chVTGetSystemTimeX() - start;
    }
    palClearPad(GPIOD, GPIOD_LED4);
    if (chThdShouldTerminateX()) chThdExit(MSG_OK);
//    end = chVTGetSystemTimeX();
    chThdSleepMilliseconds(available_ms);
  }
}

/* Define the thread function */
static THD_FUNCTION(Thread1, arg) {
    (void)arg;
    chRegSetThreadName("ecdsa_sign");

    /* Initialize the parameters for the signature */
    uint8_t message[MESSAGE_SIZE_BYTES];
    uint32_t pubkey_x[8], pubkey_y[8], privkey[8];
    uint32_t signature_r[8], signature_s[8];

    /* Initialize the parameters for cycle count and timer */
    uint32_t cycle_counter;
    uint32_t start_cycle;
    uint32_t end_cycle;
    systime_t start_time;
    systime_t end_time;
    systime_t timer;
    uint8_t loops = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;


    /* Generate key pair */
    do {
        randombytes(privkey, sizeof(privkey));
    } while (!p256_keygen(pubkey_x, pubkey_y, privkey));

    uint8_t i;
    for (i = 0; i < sizeof(available_ms_table); i++){
      available_ms = available_ms_table[i];
      loops = 0;
//      chprintf((BaseSequentialStream *)&SD2,"pk: %lu - sk: %lu\r\n", public_key, secret_key);
      while (loops<100) {
          DWT->CYCCNT = 0;
          loops ++;
          /* Generate a random message */
          for (int i = 0; i < MESSAGE_SIZE_BYTES; i++) {
              message[i] = rand() % 256;
          }

#ifdef SIGN

          /* Set the counters and launch concurrent thread */
          palSetPad(GPIOD, GPIOD_LED3);       // Orange.
          monopolize = chThdCreateStatic(monopolize_wa, sizeof(monopolize_wa), HIGHPRIO, Monopolize, NULL);
          chSysLock();
          start_time = chVTGetSystemTimeX();
          start_cycle = DWT->CYCCNT;
          chSysUnlock();

#endif

          /* Sign the message */
          uint32_t k[8]; // must be kept secret
          do {
              randombytes(k, sizeof(k));
          } while (!p256_sign(signature_r, signature_s, message, sizeof(message), privkey, k));

#ifdef SIGN

          /* Update the counter and metrics, stop the concurrent thread */
          chSysLock();
          end_cycle = DWT->CYCCNT;
          end_time = chVTGetSystemTimeX();
          chSysUnlock();
          chThdTerminate(monopolize);
          chThdWait(monopolize);
          palClearPad(GPIOD, GPIOD_LED3);     // Orange.
          cycle_counter = end_cycle - start_cycle;
          timer = end_time - start_time;

#endif

#ifdef VERIFY

          /* Set the counters and launch concurrent thread */
          palSetPad(GPIOD, GPIOD_LED3);       // Orange.
          monopolize = chThdCreateStatic(monopolize_wa, sizeof(monopolize_wa), HIGHPRIO, Monopolize, NULL);
          chSysLock();
          start_time = chVTGetSystemTimeX();
          start_cycle = DWT->CYCCNT;
          chSysUnlock();

#endif

          /* Verify the signature (for testing purposes) */
          int signature_verified = p256_verify(pubkey_x, pubkey_y, message, sizeof(message), signature_r, signature_s);

#ifdef VERIFY

          /* Update the counter and metrics, stop the concurrent thread */
          chSysLock();
          end_cycle = DWT->CYCCNT;
          end_time = chVTGetSystemTimeX();
          chSysUnlock();
          chThdTerminate(monopolize);
          chThdWait(monopolize);
          palClearPad(GPIOD, GPIOD_LED3);     // Orange.
          cycle_counter = end_cycle - start_cycle;
          timer = end_time - start_time;

#endif

          /* Display the current loop results */
          chMtxLock(&mutex);
          chprintf((BaseSequentialStream *)&SD2,"%lu,%lu,%lu,%lu,%lu\r\n", available_ms, loops, cycle_counter, timer, count);
          chMtxUnlock(&mutex);

          if (signature_verified == 0) {
            chMtxLock(&mutex);
            chprintf((BaseSequentialStream *)&SD2, "Error - Signature non verified at loop %lu \r\n", loops);
            chMtxUnlock(&mutex);
            chThdExit(MSG_RESET);
          }

      }
    if (available_ms == 1) break;
    available_ms = available_ms/2;

    }
    /* Display Final Results */
    chMtxLock(&mutex);
//    chprintf((BaseSequentialStream *)&SD2, "Test finished - Final Results \r\n\r\n - Cycles - \r\nAverage: %lu \r\nMinimum: %lu \r\nMaximum %lu \r\n\r\n - Time - \r\nAverage: %lu ms \r\nMinimum: %lu ms \r\nMaximum: %lu ms", (uint32_t) avg, min, max, (systime_t) avg_t, min_t, max_t);
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

