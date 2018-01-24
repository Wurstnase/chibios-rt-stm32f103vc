#ifndef HAL_FSMC_H
#define HAL_FSMC_H

#include "hal.h"

/**
 * @brief   Enables the mutual exclusion APIs on the FSMC bus.
 */
#if !defined(FSMC_USE_MUTUAL_EXCLUSION) || defined(__DOXYGEN__)
#define FSMC_USE_MUTUAL_EXCLUSION TRUE
#endif
/**
 * @name    FSMC bus error conditions
 * @{
 */
#define FSMC_NO_ERROR               0x00    /**< @brief No error.            */
#define FSMC_BUS_ERROR              0x01    /**< @brief Bus Error.           */
#define FSMC_TIMEOUT                0x02    /**< @brief Hardware timeout.    */
/** @} */
/**
 * @brief   Driver state machine possible states.
 */
typedef enum {
  FSMC_UNINIT = 0,    /**< Not initialized.           */
  FSMC_STOP = 1,      /**< Stopped.                   */
  FSMC_READY = 2,     /**< Ready.                     */
  FSMC_ACTIVE_TX = 3, /**< Transmitting.              */
  FSMC_ACTIVE_RX = 4, /**< Receiving.                 */
  FSMC_LOCKED = 5     /**> Bus or driver locked.      */
} fsmcstate_t;

typedef enum {
  FSMC_CMD = 0,  /**< Sending a command */
  FSMC_DATA = 1, /**< Sending data */
} fsmccmddata_t;

/**
 * @brief   Type of FSMC driver condition flags.
 */
typedef uint8_t fsmcflags_t;

typedef struct {
  /**
   * @brief WR line.
   */
  ioline_t wr;
  /**
   * @brief CS line.
   */
  ioline_t cs;
  /**
   * @brief RS line.
   */
  // ioline_t rs;
  /**
   * @brief D/C line.
   */
  ioline_t dc;
  /**
   * @brief RD line.
   */
  ioline_t rd;
  /**
   * @brief Reset line.
   */
  ioline_t reset;
  /**
   * @brief Write port.
   */
  ioportid_t wr_port;
  /**
   * @brief Write mask;
   */
  uint16_t wr_mask;
  /**
   * @brief Write offset;
   */
  uint8_t wr_offset;
} FSMCConfig;

/**
 * @brief Type of a structure representing a FSMC driver.
 */
typedef struct FSMCDriver FSMCDriver;

/**
 * @brief Structure representing a FSMC driver.
 */
struct FSMCDriver {
  /**
   * @brief Driver state.
   */
  fsmcstate_t state;
  /**
   * @brief Current configuration data.
   */
  const FSMCConfig *config;
  /**
   * @brief Error flags.
   */
  fsmcflags_t errors;
#if (FSMC_USE_MUTUAL_EXCLUSION == TRUE) || defined(__DOXYGEN__)
  mutex_t mutex;
#endif
};

/**
 * Driver macros.
 */
#define fsmc_lld_get_errors(fsmc) ((fsmc)->errors)

extern FSMCDriver FSMCD1;

void fsmcInit(void);
void fsmcObjectInit(FSMCDriver *fsmc);
void fsmcStart(FSMCDriver *fsmc, const FSMCConfig *config);
void fsmcStop(FSMCDriver *fsmc);

void fsmcWrite(FSMCDriver *fsmc, fsmccmddata_t cmddata,
               const uint8_t *txbuf, size_t txbytes);

#if (FSMC_USE_MUTUAL_EXCLUSION == TRUE) || defined(__DOXYGEN__)
void fsmcAcquireBus(FSMCDriver *fsmc);
void fsmcReleaseBus(FSMCDriver *fsmc);
#endif

#endif /* HAL_FSMC_H */
