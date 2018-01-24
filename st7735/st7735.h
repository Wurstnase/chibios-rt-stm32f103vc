#ifndef __ST7735_H__
#define __ST7735_H__

#include "hal.h"
#include "hal_fsmc.h"

/*
 * Derived constants and error checks
 */

#define ST7735_WIDTH 128
#define ST7735_HEIGHT 128

#define ST7735_NOP 0x00
#define ST7735_SWRESET 0x01
#define ST7735_RDDID 0x04
#define ST7735_RDDST 0x09

#define ST7735_SLPIN 0x10
#define ST7735_SLPOUT 0x11
#define ST7735_PTLON 0x12
#define ST7735_NORON 0x13

#define ST7735_INVOFF 0x20
#define ST7735_INVON 0x21
#define ST7735_DISPOFF 0x28
#define ST7735_DISPON 0x29
#define ST7735_CASET 0x2A
#define ST7735_RASET 0x2B
#define ST7735_RAMWR 0x2C
#define ST7735_RAMRD 0x2E

#define ST7735_PTLAR 0x30
#define ST7735_COLMOD 0x3A
#define ST7735_MADCTL 0x36

#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR 0xB4
#define ST7735_DISSET5 0xB6

#define ST7735_PWCTR1 0xC0
#define ST7735_PWCTR2 0xC1
#define ST7735_PWCTR3 0xC2
#define ST7735_PWCTR4 0xC3
#define ST7735_PWCTR5 0xC4
#define ST7735_VMCTR1 0xC5

#define ST7735_RDID1 0xDA
#define ST7735_RDID2 0xDB
#define ST7735_RDID3 0xDC
#define ST7735_RDID4 0xDD

#define ST7735_PWCTR6 0xFC

#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

/*
 * Driver data, structures and types
 */
typedef enum {
  ST7735_BLACK = 0x0000,
  ST7735_BLUE = 0x001F,
  ST7735_RED = 0xF800,
  ST7735_GREEN = 0x07E0,
  ST7735_CYAN = 0x07FF,
  ST7735_MAGENTA = 0xF81F,
  ST7735_YELLOW = 0xFFE0,
  ST7735_WHITE = 0xFFFF,
} st7735_color_t;

typedef enum {
  ST7735_UNINIT = 0,
  ST7735_STOP = 1,
  ST7735_READY = 2,
} st7735_state_t;

typedef struct {
  FSMCDriver *fsmcp;
  const FSMCConfig *fsmccfg;
} ST7735Config;

#define _st7735_methods           \
  void (*updateScreen)(void *ip); \
  void (*bufFillScreen)(void *ip, st7735_color_t color);

struct ST7735VMT {
      _st7735_methods
};

#define _st7735_data \
        st7735_state_t state; \
        const ST7735Config *config;

typedef struct ST7735Driver {
  const struct ST7735VMT *vmt;
  _st7735_data;

  uint8_t x;
  uint8_t y;
  uint8_t w;
  uint8_t h;
  uint8_t inv;
  st7735_color_t fb[ST7735_WIDTH * ST7735_HEIGHT];
} ST7735Driver;

/*
 * Driver macros
 */

#define st7735UpdateScreen(ip) \
        (ip)->vmt->updateScreen(ip)

#define st7735FillScreen(ip, color) \
        (ip)->vmt->bufFillScreen(ip, color)
/*
 * External declarations
 */

/* extern const st7735_font_t st7735_font_standard */

void st7735ObjectInit(ST7735Driver *devp);
void st7735Start(ST7735Driver *drvp, const ST7735Config *drvcfg);

#endif  /* __ST7735_H__ */