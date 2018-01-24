#include "hal.h"
#include "hal_fsmc.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/
FSMCDriver FSMCD1;

#define TIME_NS2I(ns) \
  ((((ns)*1000) * (STM32_TIMCLK1 / 1000000) / 1000000) + 1)

#define gpt_lld_change_counter(gptp, value) \
  ((gptp)->tim->CNT = value)

#define gptChangeCounterI(gptp, value) gpt_lld_change_counter(gptp, value)

/**
 * low level driver definitions
 */
static const GPTConfig gptcfg = {
  STM32_TIMCLK1,
  NULL,
  0,
  0
};

void fsmc_lld_start(FSMCDriver *fsmc) {

  palSetLineMode(fsmc->config->wr, PAL_MODE_OUTPUT_PUSHPULL);
  palSetLineMode(fsmc->config->cs, PAL_MODE_OUTPUT_PUSHPULL);
  palSetLineMode(fsmc->config->dc, PAL_MODE_OUTPUT_PUSHPULL);
  palSetLineMode(fsmc->config->rd, PAL_MODE_OUTPUT_PUSHPULL);
  palSetLineMode(fsmc->config->reset, PAL_MODE_OUTPUT_PUSHPULL);

  palSetGroupMode(fsmc->config->wr_port,
                  fsmc->config->wr_mask,
                  fsmc->config->wr_offset,
                  PAL_MODE_OUTPUT_PUSHPULL);
  /**
   * We only use 1 fsmc.
   * So CS can be low and RD must be stay high
   */
  palClearLine(fsmc->config->cs);
  palSetLine(fsmc->config->rd);

  palClearLine(fsmc->config->reset);
  chThdSleepMilliseconds(10),
  palSetLine(fsmc->config->reset);
  chThdSleepMilliseconds(50);
}

void fsmc_lld_stop(FSMCDriver *fsmc) {
  (void)fsmc;
  gptStop(&GPTD4);
}

static inline void fsmc_write_start(FSMCDriver *fsmc) {
  palClearLine(fsmc->config->wr);
}

static inline void fsmc_write_header(FSMCDriver *fsmc, fsmccmddata_t cmddata) {
  palWriteLine(fsmc->config->dc, cmddata == FSMC_DATA ? PAL_HIGH : PAL_LOW);
}

static inline void fsmc_write_stop(FSMCDriver *fsmc) {
  palClearLine(fsmc->config->wr);
}

static inline void fsmc_write_byte(FSMCDriver *fsmc, unsigned byte) {

  fsmc_write_start(fsmc);
  // gptChangeCounterI(&GPTD4, 0);

  palWriteGroup(fsmc->config->wr_port, fsmc->config->wr_mask, fsmc->config->wr_offset, byte);
  // while (gptGetCounterX(&GPTD4) <= (TIME_NS2I(30) - 1)) /* at least 30ns */
  //   ;

  fsmc_write_stop(fsmc);
  // while (gptGetCounterX(&GPTD4) <= (TIME_NS2I(100) - 1)) /* total at least 100ns*/
  //   ;
}

static inline void fsmc_lld_write(FSMCDriver *fsmc, fsmccmddata_t cmddata,
                                  const uint8_t *txbuf, size_t txbytes) {

  if (cmddata == FSMC_CMD) {
    fsmc_write_header(fsmc, FSMC_CMD);
    fsmc_write_byte(fsmc, *txbuf++);
    --txbytes;
  }

  /**
   * still something to send?
   * Then it is data
   */
  if (txbytes) {
    fsmc_write_header(fsmc, FSMC_DATA);

    do {
      fsmc_write_byte(fsmc, *txbuf++);
    } while (--txbytes);
  }
}

/**
 * driver abstactions
 */


void fsmcInit(void) {
  fsmcObjectInit(&FSMCD1);
}

void fsmcObjectInit(FSMCDriver *fsmc) {
  fsmc->state = FSMC_STOP;
  fsmc->config = NULL;

  osalMutexObjectInit(&fsmc->mutex);
}

void fsmcStart(FSMCDriver *fsmc, const FSMCConfig *config) {
  osalDbgCheck((fsmc != NULL) && (config != NULL));
  osalDbgAssert((fsmc->state == FSMC_STOP) || (fsmc->state == FSMC_READY) ||
                    (fsmc->state == FSMC_LOCKED),
                "invalid state");

  // gptStart(&GPTD4, &gptcfg);

  osalSysLock();
  fsmc->config = config;
  fsmc_lld_start(fsmc);
  fsmc->state = FSMC_READY;
  osalSysUnlock();

  // gptStartContinuous(&GPTD4, 0xFFFF);
}

void fsmcWrite(FSMCDriver *fsmc, fsmccmddata_t cmddata,
               const uint8_t *txbuf, size_t txbytes) {

  osalDbgCheck((fsmc != NULL) &&
               (txbytes > 0U) && (txbuf != NULL));

  osalSysLock();
  fsmc->errors = FSMC_NO_ERROR;
  fsmc->state = FSMC_ACTIVE_TX;

  fsmc_lld_write(fsmc, cmddata, txbuf, txbytes);

  fsmc->state = FSMC_READY;
  osalSysUnlock();
}

#if (FSMC_USE_MUTUAL_EXCLUSION == TRUE) || defined(__DOXYGEN__)
void fsmcAcquireBus(FSMCDriver *fsmc) {
  osalDbgCheck(fsmc != NULL);

  osalMutexLock(&fsmc->mutex);
}

void fsmcReleaseBus(FSMCDriver *fsmc) {
  osalDbgCheck(fsmc != NULL);

  osalMutexUnlock(&fsmc->mutex);
}
#endif