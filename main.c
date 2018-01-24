/*
    ChibiOS - Copyright (C) 2006..2016 Giovanni Di Sirio

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

#include <stdio.h>
#include <string.h>

#include "ch.h"
#include "hal.h"

#include "chprintf.h"

#include "hal_fsmc.h"
#include "st7735.h"


/*===========================================================================*/
/* Generic code.                                                             */
/*===========================================================================*/
static THD_WORKING_AREA(waSerial, 256);
static THD_FUNCTION(Serial, arg) {
  (void)arg;

  while (true) {
    chThdSleepMilliseconds(500);
  }
}

static const FSMCConfig fsmccfg = {
  PAL_LINE(GPIOB, 0U),
  PAL_LINE(GPIOA, 7U),
  PAL_LINE(GPIOC, 5U),
  PAL_LINE(GPIOB, 1U),
  PAL_LINE(GPIOC, 4U),
  GPIOE,
  0xff,
  0,
};

static const ST7735Config st7735cfg = {
  &FSMCD1,
  &fsmccfg,
};

ST7735Driver ST7735;

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
  halInit();
  chSysInit();

  palSetPadMode(GPIOD, 12U, PAL_MODE_OUTPUT_PUSHPULL);
  palWritePad(GPIOD, 12U, PAL_LOW);

  palSetPadMode(GPIOA, 9U, PAL_MODE_STM32_ALTERNATE_PUSHPULL);
  palSetPadMode(GPIOA, 10U, PAL_MODE_INPUT);

  sdStart(&SD1, NULL);

  sdWrite(&SD1, (uint8_t *)"Starting...\r\n", 13);
  
  fsmcObjectInit(&FSMCD1);
  fsmcStart(&FSMCD1, &fsmccfg);
  
  st7735ObjectInit(&ST7735);
  st7735Start(&ST7735, &st7735cfg);
  
  st7735FillScreen(&ST7735, ST7735_BLUE);
  st7735UpdateScreen(&ST7735);

  gptObjectInit(&GPTD4);
  
  /*
   * Creates the blinker thread.
   */
  chThdCreateStatic(waSerial, sizeof(waSerial), NORMALPRIO, Serial, NULL);

  /*
   * Normal main() thread activity, spawning shells.
   */
  #define TIME_NS2I(ns) \
    ((((ns)*1000UL) * (STM32_SYSCLK / 1000000UL) / 1000000UL) + 1)
  chprintf((BaseSequentialStream *)&SD1, "30ns: %d\r\n", TIME_NS2I(30));
  chprintf((BaseSequentialStream *)&SD1, "clock: %d\r\n", STM32_SYSCLK);

      while (true)
  {
    chThdSleepMilliseconds(200);
    sdPut(&SD1, '1');
    // st7735UpdateScreen(&ST7735);
  }
}
