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
#include "config.h"

/* Define the size of the message to sign */
#define MESSAGE_SIZE_BYTES 16
#define DILITHIUM_SIG_WORKBUF_SIZE 32768

/* Define the Falcon signature parameters */
#define DILITHIUM_PARAM_SET CRYPTO_SECRETKEYBYTES


/*
 * This is a periodic thread that does absolutely nothing except flashing
 * a LED.
 */


static THD_WORKING_AREA(waThread1, DILITHIUM_SIG_WORKBUF_SIZE);

/* Define the thread function */
static THD_FUNCTION(Thread1, arg) {
    (void)arg;
    chRegSetThreadName("dilithium_sign");

    /* Create a buffer to hold the message to sign */
    uint8_t message[MESSAGE_SIZE_BYTES];
    uint8_t signature[CRYPTO_BYTES];
    uint8_t public_key[CRYPTO_PUBLICKEYBYTES];
    uint8_t secret_key[CRYPTO_SECRETKEYBYTES];// = (const uint8_t*) "Y\004\0@\0041;üNÀ#ÑE\027Ñ>\v\236@\f\217\202ôo·ü\036» >ÃøOþÿÝ\201\027 ûð?ø\b\020Æ\0N|  @\004\037\003÷ÿ\177çï?$\237úð0\212$\017ÀÿÐ}+Ð\201÷ @óÐ@\003\220}\0\022ýïÐ\0\024@\004Ô^\002ûð}\003ÿý\024`\204\aß\202ü\0;\024nÿ\004!þ\aÿ¾\0oûàRC\030\016\210ð^¾\024oþóï½\0/D\003Ï?\eá~Ü Æë±Ãø\036\200\024\017|\eÏ¿\0\036E\003À\003\f\"\002\vð\211\eï¼\003ÿÃ\0!\177\020\0Ç\024\021¿ô\200ûìAFà\200F\003"...


    crypto_sign_keypair(public_key, secret_key);
//    secret:"Y\004\0@\0041;üNÀ#ÑE\027Ñ>\v\236@\f\217\202ôo·ü\036» >ÃøOþÿÝ\201\027 ûð?ø\b\020Æ\0N|  @\004\037\003÷ÿ\177çï?$\237úð0\212$\017ÀÿÐ}+Ð\201÷ @óÐ@\003\220}\0\022ýïÐ\0\024@\004Ô^\002ûð}\003ÿý\024`\204\aß\202ü\0;\024nÿ\004!þ\aÿ¾\0oûàRC\030\016\210ð^¾\024oþóï½\0/D\003Ï?\eá~Ü Æë±Ãø\036\200\024\017|\eÏ¿\0\036E\003À\003\f\"\002\vð\211\eï¼\003ÿÃ\0!\177\020\0Ç\024\021¿ô\200ûìAFà\200F\003"...
//    public:"\t´ÄI\023ç\eãg\b\0266ÄÑÌ^YéÑw$\211\037JKÅP]±¥:\022K×\230\022\216Æ\223ôÙj´§z\207hôÐjºÄ­Á¡h¿\201²\\Ã§E«eàë\202WCÃTþÔæ«TÅ\016\211\234óØCæ$â\224ê<ãì\227a¤·\237á}*L,\023L$é^)u\023Uê\001\nQ\004\025ÝÕ¹obVy\001+1\231RÍ&\\ì\214:t\230\230\002\203\203\2044\026J¿ÞF¸P+\203\0Z¹N\221Ü\005Ï¦Ï~Àm1\224Ì8Tí93ÔÅ\022B\231\211\232Þ\213Ú&ÌCòöIfV\025¡R,ÔÃ°tÞ"...
//    memset(secret_key, 0, sizeof(secret_key));
//    memset(public_key, 0, sizeof(public_key));
    chnWrite(&SD1, public_key, 690);

    while (true) {
        /* Generate a random message */
        for (int i = 0; i < MESSAGE_SIZE_BYTES; i++) {
            message[i] = rand() % 256;
        }
//        memset(message, 0, sizeof(message));
        /* Sign the message */
        chnWrite(&SD1, secret_key, 690);

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

