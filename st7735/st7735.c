#include <string.h>

#include "st7735.h"

/*
 * Driver local functions
 */

static void wrDat(void *ip, const uint8_t *txbuf, size_t txbytes) {
  const ST7735Driver *drvp = (const ST7735Driver *)ip;

  fsmcAcquireBus(drvp->config->fsmcp);
  // fsmcStart(drvp->config->fsmcp, drvp->config->fsmccfg);

  fsmcWrite(drvp->config->fsmcp, FSMC_DATA, txbuf, txbytes);

  fsmcReleaseBus(drvp->config->fsmcp);
}

static void wrCmdDat(void *ip, const uint8_t *txbuf, size_t txbytes) {
  const ST7735Driver *drvp = (const ST7735Driver *)ip;

  fsmcAcquireBus(drvp->config->fsmcp);
  // fsmcStart(drvp->config->fsmcp, drvp->config->fsmccfg);

  fsmcWrite(drvp->config->fsmcp, FSMC_CMD, txbuf, txbytes);

  fsmcReleaseBus(drvp->config->fsmcp);
}

static void setBufWindow(void *ip,
                         uint8_t x, uint8_t y,
                         uint8_t w, uint8_t h) {

  ST7735Driver *drvp = (ST7735Driver *)ip;
  drvp->x = x;
  drvp->y = y;
  drvp->w = w;
  drvp->h = h;
}

static void setAddrWindow(void *ip) {
  ST7735Driver *drvp = (ST7735Driver *)ip;
  uint8_t buf[] = {0x00, 0x00, 0x00, 0x00, 0x00};

  buf[0] = ST7735_CASET;
  buf[2] = drvp->x;
  buf[4] = drvp->w;
  wrCmdDat(drvp, buf, sizeof(buf));

  buf[0] = ST7735_RASET;
  buf[2] = drvp->y;
  buf[3] = drvp->h;
  wrCmdDat(drvp, buf, sizeof(buf));

  buf[0] = ST7735_RAMWR;
  wrCmdDat(drvp, buf, 1);
}

static void fillScreen(void *ip) {
  ST7735Driver *drvp = (ST7735Driver *)ip;

  uint8_t h = drvp->h;
  uint8_t w = drvp->w;

  uint8_t y;
  uint8_t x = drvp->x;

  for (y=h; y>0; y--) {
    wrDat(drvp, (uint8_t *)(drvp->fb + x + (y * ST7735_WIDTH)), w * 2);
  }
}

static void updateScreen(void *ip) {
  ST7735Driver *drvp = (ST7735Driver *)ip;
  sdPut(&SD1, 's');
  setAddrWindow(drvp);
  fillScreen(drvp);
  sdPut(&SD1, 'e');
}

static void bufFillRect(void *ip,
                        uint8_t x, uint8_t y,
                        uint8_t w, uint8_t h,
                        st7735_color_t color) {

  ST7735Driver *drvp = (ST7735Driver *)ip;
  if ((x >= ST7735_WIDTH) || (y >= ST7735_HEIGHT))
    return;

  if ((x + w - 1) >= ST7735_WIDTH)
    w = ST7735_WIDTH - x;
  if ((y + h - 1) >= ST7735_HEIGHT)
    h = ST7735_HEIGHT - y;

  setBufWindow(drvp, x, y, y+w-1, y+h-1);

  uint16_t row;
  row = (uint16_t)y * ST7735_WIDTH;

  for (; row < (h * ST7735_WIDTH); row+=ST7735_WIDTH) {
    memset(drvp->fb + row + x, color, w);
  }

}

static void bufFillScreen(void *ip, st7735_color_t color) {
  FSMCDriver *drvp = (FSMCDriver *)ip;

  bufFillRect(drvp, 0, 0, ST7735_WIDTH, ST7735_HEIGHT, color);
}

static const struct ST7735VMT vmt_st7735 = {
  updateScreen,
  bufFillScreen
};

/*
 * Driver export functions
 */
#define DELAY 0x80

static void wrInitList(ST7735Driver *drvp, const uint8_t *cmd_list) {
  uint8_t total_cmds, this_args;
  uint8_t ms_delay;

  total_cmds = *cmd_list++;
  while (total_cmds--) {
    this_args = *cmd_list++;
    ms_delay = this_args & DELAY;
    this_args &= ~DELAY;
    wrCmdDat(drvp, cmd_list, this_args);
    cmd_list += this_args;

    if (ms_delay) {
      ms_delay = *cmd_list++;
      chThdSleepMilliseconds(ms_delay);
    }
  }
}

#define FRMCTR_3CMD 

void st7735ObjectInit(ST7735Driver *drvp) {
  const uint8_t cmd[] = {
    20,
      DELAY + 1, ST7735_SWRESET,  /*  1: Software reset with delay */
        50,
      DELAY + 1, ST7735_SLPOUT,   /*  2: Out of sleep mode with delay */
        120,
      3 + 1, ST7735_FRMCTR1,      /*  3: Frame ctrl - normal mode */
        0x02, 0x35, 0x36,
      3 + 1, ST7735_FRMCTR2,      /*  4: Frame ctrl */
        0x02, 0x35, 0x36,
      6 + 1, ST7735_FRMCTR3,      /*  5: Frame ctrl */
        0x02, 0x35, 0x36,
        0x02, 0x35, 0x36,
      1, ST7735_INVCTR,           /*  6: Display inversion ctrl */
        0x03,
      3 + 1, ST7735_PWCTR1,       /*  7: Power control */
        0xA2, 0x02, 0x84,         /*  - , -4.6V, Auto mode */
      1 + 1, ST7735_PWCTR2,       /*  8: Power control */
        0xC5,                     /* VGH25 = 2.4C VGSEL = -10 VGH = 3*AVDD */
      2 + 1, ST7735_PWCTR3,       /*  9: Power control */
        0x0D, 0x00,
      2 + 1, ST7735_PWCTR4,       /* 10: Power control */
        0x8D, 0x2A,
      2 + 1, ST7735_PWCTR5,       /* 11: Power control */
        0x8D, 0xEE,
      1 + 1, ST7735_VMCTR1,       /* 12: Power control */
        0x09,
      1, ST7735_INVOFF,           /* 13: Invert display off */
      1 + 1, ST7735_MADCTL,       /* 14: Memory access control */
        0xC8,                     /* row addr/col addr, bottom to top refresh */
      1 + 1, ST7735_COLMOD,       /* 15: Set color mode */
        0x05,                     /* 16bit */
      16 + 1, ST7735_GMCTRP1,     /* 16: Gamma ctrl, (+) polarity corrections */
      /* Adafruit */
      /*  0x02, 0x1c, 0x07, 0x12, 
        0x37, 0x32, 0x29, 0x2d,
        0x29, 0x25, 0x2B, 0x39,
        0x00, 0x01, 0x03, 0x10, */
      /* Custom */
        0x12, 0x1C, 0x10, 0x18,
        0x33, 0x2C, 0x25, 0x28,
        0x28, 0x27, 0x2F, 0x3C,
        0x00, 0x03, 0x03, 0x10,
      16 + 1, ST7735_GMCTRN1,     /* 17: Gamma ctrl, (-) polarity corrections */
      /* Adafruit */
      /* 0x03, 0x1d, 0x07, 0x06,
        0x2E, 0x2C, 0x29, 0x2D,
        0x2E, 0x2E, 0x37, 0x3F,
        0x00, 0x00, 0x02, 0x10, */
      /* Custom */
        0x12, 0x1C, 0x10, 0x18,
        0x2D, 0x28, 0x23, 0x28,
        0x28, 0x26, 0x2F, 0x3B,
        0x00, 0x03, 0x03, 0x10,
      4 + 1, ST7735_CASET,        /* 18: Column addr set */
        0x00, 0x00,               /* XStart = 0 */
        0x00, 0x7F,               /* XEND = 127 */
      4 + 1, ST7735_RASET,        /* 19: Row addr set */
        0x00, 0x00,               /* YSTART = 0 */
        0x00, 0x7F,               /* YEND = 127 */
      DELAY + 1, ST7735_DISPON,   /* 20: Main screen on with delay */
        100,
    };

  wrInitList(drvp, cmd);
  
  drvp->vmt = &vmt_st7735;
  drvp->config = NULL;

  drvp->state = ST7735_STOP;
}

void st7735Start(ST7735Driver *drvp, const ST7735Config *drvcfg) {
  drvp->config = drvcfg;
  drvp->state = ST7735_READY;
}
// 